#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct Voter {
    std::string voterId;
    std::string name;
    std::string phone;
    std::string password;
};

struct Candidate {
    std::string candidateId;
    std::string candidateName;
    std::string partyName;
    std::string electionId;
    std::string description;
    std::string photoUrl;
};

struct Election {
    std::string electionId;
    std::string electionTitle;
    std::string startDate;
    std::string endDate;
    std::string electionStatus;
    std::string resultsPublished;
};

struct Vote {
    std::string voteId;
    std::string electionId;
    std::string voterId;
    std::string candidateId;
    std::string timestamp;
};

class Database {
private:
    std::string votersFile;
    std::string candidatesFile;
    std::string electionsFile;
    std::string votesFile;

    // Helper to split a string by delimiter
    std::vector<std::string> split(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::stringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    Database(std::string vFile, std::string cFile, std::string eFile, std::string voteFile)
        : votersFile(vFile), candidatesFile(cFile), electionsFile(eFile), votesFile(voteFile) {}

    // --- Voters ---
    std::vector<Voter> getAllVoters() {
        std::vector<Voter> voters;
        std::ifstream file(votersFile);
        if (!file.is_open()) {
            // Try parent directory if relative path fails
            file.open("../" + votersFile);
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 4) {
                voters.push_back({parts[0], parts[1], parts[2], parts[3]});
            }
        }
        return voters;
    }

    bool registerVoter(const Voter& voter) {
        auto voters = getAllVoters();
        for (const auto& v : voters) {
            if (v.voterId == voter.voterId || v.phone == voter.phone) {
                return false; // Already exists
            }
        }
        std::ofstream file(votersFile, std::ios::app);
        if (!file.is_open()) {
            file.open("../" + votersFile, std::ios::app);
        }
        if (file.is_open()) {
            file << voter.voterId << "|" << voter.name << "|" << voter.phone << "|" << voter.password << "\n";
            return true;
        }
        return false;
    }

    Voter* authenticateVoter(const std::string& voterId, const std::string& password) {
        static Voter foundVoter;
        auto voters = getAllVoters();
        for (auto& v : voters) {
            if (v.voterId == voterId && v.password == password) {
                foundVoter = v;
                return &foundVoter;
            }
        }
        return nullptr;
    }

    // --- Candidates ---
    std::vector<Candidate> getAllCandidates() {
        std::vector<Candidate> candidates;
        std::ifstream file(candidatesFile);
        if (!file.is_open()) {
            file.open("../" + candidatesFile);
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 6) {
                candidates.push_back({parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]});
            } else if (parts.size() == 5) {
                // Fallback if photoUrl is missing in some legacy lines
                candidates.push_back({parts[0], parts[1], parts[2], parts[3], parts[4], ""});
            }
        }
        return candidates;
    }

    // --- Elections ---
    std::vector<Election> getAllElections() {
        std::vector<Election> elections;
        std::ifstream file(electionsFile);
        if (!file.is_open()) {
            file.open("../" + electionsFile);
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 6) {
                elections.push_back({parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]});
            }
        }
        return elections;
    }

    // --- Votes ---
    std::vector<Vote> getAllVotes() {
        std::vector<Vote> votes;
        std::ifstream file(votesFile);
        if (!file.is_open()) {
            file.open("../" + votesFile);
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            auto parts = split(line, '|');
            if (parts.size() >= 5) {
                votes.push_back({parts[0], parts[1], parts[2], parts[3], parts[4]});
            }
        }
        return votes;
    }

    bool hasVoted(const std::string& voterId, const std::string& electionId) {
        auto votes = getAllVotes();
        for (const auto& v : votes) {
            if (v.voterId == voterId && v.electionId == electionId) {
                return true;
            }
        }
        return false;
    }

    bool castVote(const Vote& vote) {
        if (hasVoted(vote.voterId, vote.electionId)) return false;
        std::ofstream file(votesFile, std::ios::app);
        if (!file.is_open()) {
            file.open("../" + votesFile, std::ios::app);
        }
        if (file.is_open()) {
            file << vote.voteId << "|" << vote.electionId << "|" << vote.voterId << "|" << vote.candidateId << "|" << vote.timestamp << "\n";
            return true;
        }
        return false;
    }
};

#endif