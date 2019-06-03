package com.ttrs.ticketbookingapp.Train;

public class Station {

    protected String loc, time;
    protected String[] price;

    public Station(String loc, String time, String[] price) {
        this.loc = loc;
        this.time = time;
        this.price = price;
    }

}
