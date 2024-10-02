// interpreter.js
const readline = require('readline');
const { exec } = require("child_process");

// Create an interface for reading from stdin
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

// Function to interact with the C program
function runCProgram(input) {
    return new Promise((res) => 
        exec(`.\\skrotoskript2.exe -c ${input.replace(/"/g, '\\"')}`, (error, stdout, stderr) => res(stdout))
    );
}

// Function to read and process input in a loop
function mainLoop() {
    rl.question('|> ', async (input) => {
        try {
            let output = await runCProgram(input);
            console.log('|' + output.replace(/\n/g, '\n|'));
        } catch (error) {
            console.error(`Error: ${error.message}`);
        }

        // Repeat the process
        mainLoop();
    });
}

// Start the main loop
console.log("|\n|-- SKROTOSKRIPT 2.0 INTERPRETER --|\n|");
mainLoop();
