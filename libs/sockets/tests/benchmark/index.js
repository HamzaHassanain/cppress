const http = require("http");
const fs = require("fs");
const path = require("path");

// Configuration
const HOST = "localhost";
const PORT = 8080; // Adjust this to match your server port
const CONCURRENT_REQUESTS = 100; // Number of concurrent requests
const TOTAL_REQUESTS = 1000; // Total requests to send

// Read the data file
const dataPath = path.join(__dirname, "data.json");
const data = fs.readFileSync(dataPath);
const dataSize = data.length;

console.log(`Data file size: ${dataSize} bytes`);
console.log(
  `Sending ${TOTAL_REQUESTS} requests with ${CONCURRENT_REQUESTS} concurrent connections...`
);

// Statistics
let completedRequests = 0;
let successfulRequests = 0;
let failedRequests = 0;
let startTime = Date.now();
let responseSizes = [];

// Function to send a single request
function sendRequest(requestId) {
  return new Promise((resolve, reject) => {
    const options = {
      hostname: HOST,
      port: PORT,
      path: "/stress-test",
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "Content-Length": dataSize,
        "X-Request-ID": requestId.toString(),
      },
    };

    const req = http.request(options, (res) => {
      let responseBody = "";

      res.on("data", (chunk) => {
        responseBody += chunk;
      });

      res.on("end", () => {
        completedRequests++;

        // Parse JSON response to extract the length
        try {
          const responseJson = JSON.parse(responseBody.trim());
          const responseSize = responseJson.length;
          responseSizes.push(responseSize);

          if (responseSize >= dataSize) {
            successfulRequests++;
            // console.log(
            //   `✓ Request ${requestId}: Response size ${responseSize} >= ${dataSize} (OK)`
            // );
          } else {
            failedRequests++;
            // console.log(
            //   `✗ Request ${requestId}: Response size ${responseSize} < ${dataSize} (FAILED)`
            // );
          }
        } catch (err) {
          failedRequests++;
          console.log(
            `✗ Request ${requestId}: Invalid JSON response - ${err.message}`
          );
        }

        resolve();
      });
    });

    req.on("error", (err) => {
      completedRequests++;
      failedRequests++;
      console.log(`✗ Request ${requestId}: Error - ${err.message}`);
      resolve();
    });

    // Send the data
    req.write(data);
    req.end();
  });
}

// Function to run stress test with concurrency control
async function runStressTest() {
  console.log(`Starting stress test at ${new Date().toISOString()}`);

  const promises = [];
  let requestId = 0;

  // Create batches of concurrent requests
  for (let i = 0; i < TOTAL_REQUESTS; i += CONCURRENT_REQUESTS) {
    const batch = [];
    for (let j = 0; j < CONCURRENT_REQUESTS && i + j < TOTAL_REQUESTS; j++) {
      batch.push(sendRequest(++requestId));
    }

    // Wait for current batch to complete before starting next batch
    await Promise.all(batch);
  }

  // Wait for all requests to complete
  while (completedRequests < TOTAL_REQUESTS) {
    await new Promise((resolve) => setTimeout(resolve, 100));
  }

  const endTime = Date.now();
  const duration = (endTime - startTime) / 1000;

  // Print results
  console.log("\n=== STRESS TEST RESULTS ===");
  console.log(`Total requests: ${TOTAL_REQUESTS}`);
  console.log(`Successful requests: ${successfulRequests}`);
  console.log(`Failed requests: ${failedRequests}`);
  console.log(`Duration: ${duration.toFixed(2)} seconds`);
  console.log(`Requests per second: ${(TOTAL_REQUESTS / duration).toFixed(2)}`);
  console.log(
    `Average response size: ${(
      responseSizes.reduce((a, b) => a + b, 0) / responseSizes.length
    ).toFixed(0)} bytes`
  );

  if (failedRequests === 0) {
    console.log(
      "🎉 All requests passed! Server handled the load successfully."
    );
    process.exit(0);
  } else {
    console.log("❌ Some requests failed. Check server implementation.");
    process.exit(1);
  }
}

// Handle process termination
process.on("SIGINT", () => {
  console.log("\nStress test interrupted by user");
  process.exit(1);
});

// Run the stress test
runStressTest().catch((err) => {
  console.error("Stress test failed:", err);
  process.exit(1);
});
