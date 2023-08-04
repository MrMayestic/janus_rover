import cors from "cors";
import express from "express";

const app = express();

import bodyParser from "body-parser";
import mysql from "mysql";
import * as routes from "./routes/app.routes.js";
import dotenv from "dotenv"

dotenv.config();

console.log(process.env.HOST);

app.use(cors());

app.use(express.urlencoded({ extended: true }));
app.use(express.json());

app.use(bodyParser.raw({type: 'application/octet-stream', limit: "10mb"}))

app.use(bodyParser.json());
app.use(bodyParser.text());


app.use("/",routes.setRoutes());

app.listen(3000, () => {
  console.log("Listening on port 3000");
});

var db = mysql.createPool({
  connectionLimit: 100,
  acquireTimeout: 20000,
  host: process.env.HOST,
  user: process.env.USER,
  password: process.env.PASSWORD,
  database: process.env.DATABASE,
});

export default db