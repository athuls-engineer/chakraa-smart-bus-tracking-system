import { NextFunction, Response } from "express";
import prisma from "../utils/prisma";
import jwt from "jsonwebtoken";

export const isAuthenticated = (
  req: any,
  res: Response,
  next: NextFunction
) => {
  try {
    const authHeader = req.headers.authorization;
    if (!authHeader) {
      return res
        .status(401)
        .json({ message: "Please Log in to access this content!" });
    }
    const token = authHeader.split(" ")[1];
    if (!token) {
      return res.status(401).json({ message: "Token missing" });
    }

    jwt.verify(
      token,
      process.env.ACCESS_TOKEN_SECRET!,
      async (err: any, decoded: any) => {
        if (err) {
          return res.status(401).json({ message: "Invalid or expired token" });
        }

        try {
          const userData = await prisma.user.findUnique({
            where: {
              id: decoded.id,
            },
          });

          if (!userData) {
            return res.status(401).json({ message: "User account not found" });
          }

          req.user = userData;
          next();
        } catch (dbError) {
          console.error("Database lookup error in auth middleware:", dbError);
          return res.status(500).json({ message: "Internal server error during authentication" });
        }
      }
    );
  } catch (error) {
    console.error("Authentication middleware failure:", error);
    return res.status(500).json({ message: "Internal server error" });
  }
};

export const isAuthenticatedDriver = (
  req: any,
  res: Response,
  next: NextFunction
) => {
  try {
    const authHeader = req.headers.authorization;
    if (!authHeader) {
      return res
        .status(401)
        .json({ message: "Please Log in to access this content!" });
    }
    const token = authHeader.split(" ")[1];
    if (!token) {
      return res.status(401).json({ message: "Token missing" });
    }

    jwt.verify(
      token,
      process.env.ACCESS_TOKEN_SECRET!,
      async (err: any, decoded: any) => {
        if (err) {
          return res.status(401).json({ message: "Invalid or expired token" });
        }

        try {
          const driverData = await prisma.driver.findUnique({
            where: {
              id: decoded.id,
            },
          });

          if (!driverData) {
            return res.status(401).json({ message: "Driver account not found" });
          }

          req.driver = driverData;
          next();
        } catch (dbError) {
          console.error("Database lookup error in driver auth middleware:", dbError);
          return res.status(500).json({ message: "Internal server error during authentication" });
        }
      }
    );
  } catch (error) {
    console.error("Driver authentication middleware failure:", error);
    return res.status(500).json({ message: "Internal server error" });
  }
};
