function sendMessage() {
    const message = document.getElementById("message").value;
    const method = document.getElementById("method").value;
    const password = document.getElementById("password").value;

    // 🔐 Frontend authentication check
    if (password !== "admin") {
        alert("❌ Unauthorized Access");
        return;
    }

    if (message === "") {
        alert("Please enter a message");
        return;
    }

    // 🌐 Send data to backend
    fetch('/send', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },

        // 👉 THIS is where password is added
        body: JSON.stringify({
            method: method,
            message: message,
            password: password
        })
    })
    .then(res => res.json())
    .then(data => {
        // Show output
        document.getElementById("output").textContent = JSON.stringify(data, null, 2);

        // Add log
        const logs = document.getElementById("logs");
        const logEntry = document.createElement("p");
        logEntry.textContent = `[INFO] ${data.status}`;
        logs.appendChild(logEntry);
    })
    .catch(err => console.error(err));

    // Clear message field
    document.getElementById("message").value = "";
}