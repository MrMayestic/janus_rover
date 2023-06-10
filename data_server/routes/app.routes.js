import * as controllers from "../controllers/app.controllers.js";
import express from "express";
import multer  from "multer";

const router = express.Router();

const storage = multer.memoryStorage(); // Przechowuj dane w pamięci jako dane binarne
const upload = multer({ storage });


export function setRoutes() { 
    router.post("/data", controllers.postData);
    router.use("/data", controllers.useData);
    router.get("/chartData", controllers.chartData);
    router.post("/image",controllers.saveImage)
    return router;
 };