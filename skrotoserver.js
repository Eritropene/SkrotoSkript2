const express = require('express');
const { exec } = require("child_process");
const app = express();
const port = 666;

// Function to interact with the C program
function runCProgram(input) {
    return new Promise((res) => 
        exec(`.\\skrotoskript2.exe -c ${'"' + input.replace(/"/g, '""') + '"'}`, (error, stdout, stderr) => res(stdout))
    );
}

// Route to handle GET requests
app.get('/skr', (req, res) => {
  // Check if the 'code' query parameter exists
  if (req.query.code) {
    runCProgram(req.query.code).then(r => res.send(r));
  } else {
    res.send('Code parameter is missing!');
  }
});

// Start the server on localhost:666
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
