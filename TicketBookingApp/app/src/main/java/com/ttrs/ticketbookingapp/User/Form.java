package com.ttrs.ticketbookingapp.User;

import android.util.Patterns;

public class Form {

    static public boolean isValidId(String str) {
        String regex = "^[0-9]+$";
        return str.matches(regex);
    }

    static public boolean isValidPassword(String str) {
        return str != null && str.length() > 5;
    }

    static public boolean isValidEmail(String str) {
        return Patterns.EMAIL_ADDRESS.matcher(str).matches();
    }

    static public boolean isValidPhone(String str) {
        return Patterns.PHONE.matcher(str).matches();
    }

    static public boolean isValidUsername(String str) {
        return str != null;
    }

}
