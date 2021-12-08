var express = require("express");
var app = express();
var server = require("http").createServer(app);
var io = require("socket.io")(server);

var light = { state: false };

app.use(express.static(__dirname + "/node_modules"));
app.get("/", function (req, res, next) {
    res.sendFile(__dirname + "/index.html");
});

io.on("connection", function (client) {
    console.log("Client connected...");

    // handle client connection
    client.on("join", function (data) {
        console.log(data);
    });

    // handle client disconnection
    client.on("disconnect", function () {
        console.log("Client disconnected!");
    });

    io.sockets.emit("led", light);
    client.on("toggle", function (state) {
        light.state = !light.state;
        console.log("id: " + client.id + " light: " + light.state);
        io.sockets.emit("led", light);
    });
});

server.listen(8000, () => {
    console.log("http://localhost:8000");
});
