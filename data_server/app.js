const express = require("express");
const app = express();
const bodyParser = require("body-parser");
const mysql = require("mysql");

require('dotenv').config();

console.log(process.env.HOST);

var con = mysql.createConnection({
  host: process.env.HOST,
  user: process.env.USER,
  password: process.env.PASSWORD,
  database: process.env.DATABASE
});

con.connect(function (err) {
  if (err) throw err;
  console.log("Connected!");
});

app.use(express.urlencoded({ extended: true }));
app.use(express.json());
app.use(bodyParser.json());
app.use(bodyParser.text());
app.use(
  bodyParser.urlencoded({
    extended: true,
  })
);

app.post("/sendData", (req, res) => {
  console.log(req.body);

  let parseData = JSON.parse(req.body);

  var sql = "INSERT INTO jr_data (temperature, humidity, since_start) VALUES ?";
  var values = [
    [parseData["temperature"], parseData["humidity"], parseData["since_start"]],
  ];

  for (let i of values) {
    console.log(i);
  }

  con.query(sql, [values], function (err, result) {
    if (err) throw err;
    console.log("Number of records inserted: " + result.affectedRows);
  });
});

app.listen(8080, () => {
  console.log("Listening on port 8080");
});
