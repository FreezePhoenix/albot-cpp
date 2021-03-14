var child_process = require("child_process");
 var childProcess = child_process.spawn("./albot-cpp", [], {
            stdio: [0, 1, 2, 'ipc'],
 });
