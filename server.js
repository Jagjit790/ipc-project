const express = require('express');
const { execFile } = require('child_process');

const app = express();

app.use(express.json());
app.use(express.static(__dirname));

app.post('/send', (req, res) => {
    const { method, message, password } = req.body;

    // 🔐 Authentication
    if (password !== "admin") {
        return res.status(401).json({
            status: "Access Denied ❌"
        });
    }

    const start = Date.now();

    // Escape message
    const safeMessage = message.replace(/'/g, "\\'");

    execFile('wsl', [
        'bash',
        '-c',
        `cd /mnt/c/Users/asua/OneDrive/Documents/OS_Project && ./ipc ${method} '${safeMessage}'`
    ], (err, stdout, stderr) => {

        const end = Date.now();
        const timeTaken = end - start;

        if (err) {
            return res.json({
                status: "Error ❌",
                error: err.message
            });
        }

        res.json({
            method: method.toUpperCase(),
            output: stdout.trim(),
            executionTime: timeTaken + " ms",
            status: "Executed via C system calls"
        });
    });
});

app.listen(3000, () => {
    console.log("Server running on http://localhost:3000");
});