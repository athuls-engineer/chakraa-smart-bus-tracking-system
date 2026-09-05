require("dotenv").config();

import { NextFunction, Request, Response } from "express";
import twilio from "twilio";
import prisma from "../utils/prisma";
import jwt from "jsonwebtoken";
import { nylas } from "../app";
import { sendToken } from "../utils/send-token";
import { formatAndValidatePhone } from "../utils/phone.utils";

const accountSid = process.env.TWILIO_ACCOUNT_SID;
const authToken = process.env.TWILIO_AUTH_TOKEN;

const client = (accountSid && authToken)
  ? twilio(accountSid, authToken, { lazyLoading: true })
  : (null as any);

/* ===================================================== */
/* 🔥 REGISTER USER (PHONE OTP) */
/* ===================================================== */
export const registerUser = async (
  req: Request,
  res: Response,
  next: NextFunction
) => {
  try {
    let { phone_number } = req.body;

    const formatted_phone = formatAndValidatePhone(phone_number);
    console.log("📱 registerUser phone:", formatted_phone);

    await client.verify.v2
      ?.services(process.env.TWILIO_SERVICE_SID!)
      .verifications.create({
        channel: "sms",
        to: formatted_phone,
      });

    res.status(201).json({
      success: true,
    });
  } catch (error: any) {
    console.log("❌ TWILIO ERROR:", error);

    res.status(400).json({
      success: false,
      message:
        error.message ||
        "Invalid phone number format. Use 10 digits e.g. 9876543210",
    });
  }
};

/* ===================================================== */
/* 🔥 VERIFY PHONE OTP */
/* ===================================================== */
export const verifyOtp = async (
  req: Request,
  res: Response,
  next: NextFunction
) => {
  try {
    let { phone_number, otp } = req.body;

    const formatted_phone = formatAndValidatePhone(phone_number);
    console.log("📱 verifyOtp phone:", formatted_phone);
    console.log("📥 OTP:", otp);

    await client.verify.v2
      .services(process.env.TWILIO_SERVICE_SID!)
      .verificationChecks.create({
        to: formatted_phone,
        code: otp,
      });

    const isUserExist = await prisma.user.findUnique({
      where: {
        phone_number: formatted_phone,
      },
    });

    if (isUserExist) {
      await sendToken(isUserExist, res);
    } else {
      const user = await prisma.user.create({
        data: {
          phone_number: formatted_phone,
        },
      });

      res.status(200).json({
        success: true,
        message: "OTP verified successfully!",
        user,
      });
    }
  } catch (error: any) {
    console.log("❌ VERIFY OTP ERROR:", error);

    res.status(400).json({
      success: false,
      message: error.message || "Invalid OTP or phone number",
    });
  }
};

/* ===================================================== */
/* 🔥 SEND EMAIL OTP */
/* ===================================================== */
export const sendingOtpToEmail = async (
  req: Request,
  res: Response,
  next: NextFunction
) => {
  try {
    const { email, name, userId } = req.body;

    const otp = Math.floor(1000 + Math.random() * 9000).toString();

    console.log("📧 Generated OTP:", otp);

    const user = {
      userId,
      name,
      email,
    };

    const token = jwt.sign(
      {
        user,
        otp,
      },
      process.env.EMAIL_ACTIVATION_SECRET!,
      {
        expiresIn: "5m",
      }
    );

    await nylas.messages.send({
      identifier: process.env.USER_GRANT_ID!,
      requestBody: {
        to: [{ name: name, email: email }],
        subject: "Verify your email address!",
        body: `
        <p>Hi ${name},</p>
        <p>Your Chakraa verification code is <b>${otp}</b>.</p>
        <p>If you didn't request this, ignore it.</p>
        <p>Thanks,<br/>Chakraa Team</p>
        `,
      },
    });

    res.status(201).json({
      success: true,
      token,
    });
  } catch (error: any) {
    console.log("❌ EMAIL OTP ERROR:", error);

    res.status(400).json({
      success: false,
      message: error.message,
    });
  }
};

/* ===================================================== */
/* 🔥 VERIFY EMAIL OTP (FIXED 🔥🔥🔥) */
/* ===================================================== */
export const verifyingEmail = async (
  req: Request,
  res: Response,
  next: NextFunction
) => {
  try {
    const { otp, token } = req.body;

    console.log("📥 Incoming OTP:", otp);

    const newUser: any = jwt.verify(
      token,
      process.env.EMAIL_ACTIVATION_SECRET!
    );

    console.log("📦 Stored OTP:", newUser.otp);

    /* 🔥 FIX: SAFE STRING COMPARISON */
    if (String(newUser.otp) !== String(otp)) {
      return res.status(400).json({
        success: false,
        message: "OTP is not correct or expired!",
      });
    }

    const { name, email, userId } = newUser.user;

    const user = await prisma.user.findUnique({
      where: { id: userId },
    });

    if (!user) {
      return res.status(400).json({
        success: false,
        message: "User not found",
      });
    }

    /* 🔥 ALWAYS UPDATE USER */
    const updatedUser = await prisma.user.update({
      where: { id: userId },
      data: {
        name,
        email,
      },
    });

    await sendToken(updatedUser, res);
  } catch (error) {
    console.log("❌ VERIFY EMAIL ERROR:", error);

    res.status(400).json({
      success: false,
      message: "OTP expired or invalid!",
    });
  }
};

/* ===================================================== */
/* 🔥 GET USER DATA */
/* ===================================================== */
export const getLoggedInUserData = async (req: any, res: Response) => {
  try {
    res.status(200).json({
      success: true,
      user: req.user,
    });
  } catch (error) {
    console.log(error);
  }
};

/* ===================================================== */
/* 🔥 GET USER RIDES */
/* ===================================================== */
export const getAllRides = async (req: any, res: Response) => {
  try {
    const rides = await prisma.rides.findMany({
      where: {
        userId: req.user?.id,
      },
      include: {
        driver: true,
        user: true,
      },
    });

    res.status(200).json({
      rides,
    });
  } catch (error) {
    console.log(error);
  }
};