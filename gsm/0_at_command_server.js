const dgram = require('dgram');
const server = dgram.createSocket('udp4');

server.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.on('message', on_message_callback);

function on_message_callback(msg, rinfo) {
    console.log(`server got: ==>${msg}<== from ${rinfo.address}:${rinfo.port}`);
}

server.on('listening', () => {
  const address = server.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

function my_function(a, b) {
    return a*b
}

var obj = {
    firstName: "123456789",
    lastName: "Godachi",
    fullName: function xyz () {
        return this.firstName + " " + this.lastName;
    }
}

server.bind(5555);
// Prints: server listening 0.0.0.0:41234
