import db from "../app.js";
import fs from "fs";

import path, { dirname } from "path";
import { fileURLToPath } from "url";

const __dirname = dirname(fileURLToPath(import.meta.url));

export async function postData(req, res) {
  console.log(req.body);

  let parseData = JSON.parse(req.body);

  var sql = "INSERT INTO jr_data (temperature, humidity, since_start) VALUES ?";

  var values = [
    [parseData["temperature"], parseData["humidity"], parseData["since_start"]],
  ];

  sql = sql + values[0].toString();

  db.query(sql, (err, result) => {
    if (err) throw err;
    return res.send("Number of records inserted: " + result.affectedRows);
  });
}

export function useData(req, res) {
  console.log("pies");
  return res.send("GET_data");
}

export async function chartData(req, res) {
  console.log("GET_chart");

  let dbResults;

  let cb = (err, results) => {
    if (err) throw err;
    console.log(results, "cb");
    dbResults = results;

    res.set();

    return res.send(dbResults);
  };

  let sql =
    "SELECT temperature,humidity,date_time FROM jr_data ORDER BY date_time";
  db.query(sql, cb);
}

export async function saveImage(req, res) {
  console.log("saveImage");
  var photo = req.body;
  const buff = db.escape(req.body); // Node.js Buffer
  console.log();
  db.query("INSERT INTO `photos` (photo) VALUES (Binary("+buff+"))",(err, results) => {
    if (err) throw err;
    res.send(results);
  });
}


export async function deleteData(req, res) {
  db.query("INSERT INTO `photos` (photo) VALUES (Binary("+buff+"))",(err, results) => {
    if (err) throw err;
    res.send(results);
  });
}