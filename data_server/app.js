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

var db = mysql.createConnection({
  host: process.env.HOST,
  user: process.env.USER,
  password: process.env.PASSWORD,
  database: process.env.DATABASE,

  queryFormat: function (query, values) {
    if (!values) return query;
    return query.replace(
    /\:(\w+)/g,
     function (txt, key) {
      if (values.hasOwnProperty(key)) {
      return this.escape(values[key]);
    }
    return txt;
     }.bind(this)
    );
  }
});

db.connect(function (err) {
  if (err) {
    console.log("Error \n", err);
  } else {
    console.log("Connected!");
  }
});

export default db