document.addEventListener('DOMContentLoaded', () => {
    // Login Handler
    const loginForm = document.getElementById('login-form');
    if (loginForm) {
        loginForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const voterId = document.getElementById('login-voterId').value;
            const password = document.getElementById('login-password').value;

            try {
                const res = await fetch('/api/login', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ voterId, password })
                });
                const data = await res.json();
                if (res.ok && data.success) {
                    localStorage.setItem('voterId', data.voterId);
                    localStorage.setItem('voterName', data.name);
                    window.location.href = 'dashboard.html';
                } else {
                    alert(data.message || 'Login failed');
                }
            } catch (err) {
                alert('Connection error');
            }
        });
    }

    // Register Handler
    const registerForm = document.getElementById('register-form');
    if (registerForm) {
        registerForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const voterId = document.getElementById('reg-voterId').value;
            const name = document.getElementById('reg-name').value;
            const phone = document.getElementById('reg-phone').value;
            const password = document.getElementById('reg-password').value;

            try {
                const res = await fetch('/api/register', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ voterId, name, phone, password })
                });
                const data = await res.json();
                if (res.ok && data.success) {
                    alert('Registration successful! Please login.');
                    document.getElementById('register-section').style.display = 'none';
                    document.getElementById('login-section').style.display = 'block';
                    registerForm.reset();
                } else {
                    alert(data.message || 'Registration failed');
                }
            } catch (err) {
                alert('Connection error');
            }
        });
    }
});