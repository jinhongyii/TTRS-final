package com.ttrs.ticketbookingapp.Utility;

import androidx.annotation.NonNull;

public class Location {
    public String pinyin;
    public String loc;

    public Location(String pinyin, String loc) {
        this.pinyin = pinyin;
        this.loc = loc;
    }

    @NonNull
    @Override
    public String toString() {
        return loc;
    }
}
