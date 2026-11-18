#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include "httplib.h"
#include "../database/database.h"

// Helper to extract JSON values manually
std::string extractJsonField(const std::string& json, const std::string& field) {
    std::string key = "\"" + field + "\":";
    size_t pos = json.find(key);
    if (pos == std::string::npos) return "";
    pos += key.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\"' || json[pos] == '\'' || json[pos] == ':')) pos++;
    size_t endPos = pos;
    while (endPos < json.length() && json[endPos] != '\"' && json[endPos] != '\'' && json[endPos] != ',' && json[endPos] != '}') endPos++;
    return json.substr(pos, endPos - pos);
}

// Helper to get current timestamp string
std::string getCurrentTimestamp() {
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}

int main() {
    httplib::Server svr;

    // Initialize Database pointing to your 'database/' folder structure
    Database db("database/voters.txt", "database/candidates.txt", "database/elections.txt", "database/votes.txt");

    // 1. Register API
    svr.Post("/api/register", [&db](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[API] POST /api/register hit" << std::endl;
        std::string voterId = extractJsonField(req.body, "voterId");
        std::string name = extractJsonField(req.body, "name");
        std::string phone = extractJsonField(req.body, "phone");
        std::string password = extractJsonField(req.body, "password");

        if (voterId.empty() || password.empty() || name.empty()) {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"Missing required fields\"}", "application/json");
            return;
        }

        Voter newVoter = {voterId, name, phone, password};
        if (db.registerVoter(newVoter)) {
            res.set_content("{\"success\": true, \"message\": \"Registration successful\"}", "application/json");
        } else {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"Voter ID already registered\"}", "application/json");
        }
    });

    // 2. Login API
    svr.Post("/api/login", [&db](const httplib::Request& req, httplib::Response& res) {
        std::cout << "[API] POST /api/login hit" << std::endl;
        std::string voterId = extractJsonField(req.body, "voterId");
        std::string password = extractJsonField(req.body, "password");

        Voter* voter = db.authenticateVoter(voterId, password);
        if (voter != nullptr) {
            std::string jsonResp = "{\"success\": true, \"voterId\": \"" + voter->voterId + "\", \"name\": \"" + voter->name + "\"}";
            res.set_content(jsonResp, "application/json");
        } else {
            res.status = 401;
            res.set_content("{\"success\": false, \"message\": \"Invalid credentials\"}", "application/json");
        }
    });

    // 3. Elections API
    svr.Get("/api/elections", [&db](const httplib::Request& req, httplib::Response& res) {
        auto elections = db.getAllElections();
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < elections.size(); ++i) {
            ss << "{"
               << "\"electionId\": \"" << elections[i].electionId << "\","
               << "\"electionTitle\": \"" << elections[i].electionTitle << "\","
               << "\"electionStatus\": \"" << elections[i].electionStatus << "\""
               << "}";
            if (i + 1 < elections.size()) ss << ",";
        }
        ss << "]";
        res.set_content(ss.str(), "application/json");
    });

    // 4. Candidates API
    svr.Get("/api/candidates", [&db](const httplib::Request& req, httplib::Response& res) {
        auto candidates = db.getAllCandidates();
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < candidates.size(); ++i) {
            ss << "{"
               << "\"candidateId\": \"" << candidates[i].candidateId << "\","
               << "\"candidateName\": \"" << candidates[i].candidateName << "\","
               << "\"partyName\": \"" << candidates[i].partyName << "\","
               << "\"electionId\": \"" << candidates[i].electionId << "\","
               << "\"description\": \"" << candidates[i].description << "\""
               << "}";
            if (i + 1 < candidates.size()) ss << ",";
        }
        ss << "]";
        res.set_content(ss.str(), "application/json");
    });

    // 5. Vote API
    svr.Post("/api/vote", [&db](const httplib::Request& req, httplib::Response& res) {
        std::string voterId = extractJsonField(req.body, "voterId");
        std::string electionId = extractJsonField(req.body, "electionId");
        std::string candidateId = extractJsonField(req.body, "candidateId");

        std::string voteId = "V_" + voterId + "_" + electionId;
        Vote newVote = {voteId, electionId, voterId, candidateId, getCurrentTimestamp()};

        if (db.castVote(newVote)) {
            res.set_content("{\"success\": true, \"message\": \"Vote recorded\"}", "application/json");
        } else {
            res.status = 400;
            res.set_content("{\"success\": false, \"message\": \"Already voted or invalid\"}", "application/json");
        }
    });

    // Mount static frontend folder LAST so API routes take precedence
    svr.set_mount_point("/", "frontend");

    std::cout << "Voting Management System Server started at http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}