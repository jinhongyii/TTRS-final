package com.ttrs.ticketbookingapp.Ticket;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.List;

class Seat implements Parcelable {
    String kind, price;
    int numLeft;

    Seat(String kind, String price, int numLeft) {
        this.kind = kind;
        this.price = price;
        this.numLeft = numLeft;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.kind);
        dest.writeString(this.price);
        dest.writeInt(this.numLeft);
    }

    protected Seat(Parcel in) {
        this.kind = in.readString();
        this.price = in.readString();
        this.numLeft = in.readInt();
    }

    public static final Parcelable.Creator<Seat> CREATOR = new Parcelable.Creator<Seat>() {
        @Override
        public Seat createFromParcel(Parcel source) {
            return new Seat(source);
        }

        @Override
        public Seat[] newArray(int size) {
            return new Seat[size];
        }
    };
}

public class Ticket implements Parcelable {
    protected String id;
    protected String fromLoc, fromTime;
    protected String toLoc, toTime;
    protected List<Seat> seatList;

    Ticket(String id,
           String fromLoc, String fromTime,
           String toLoc, String toTime,
           List<Seat> seatList) {
        this.id = id;
        this.fromLoc = fromLoc; this.fromTime = fromTime;
        this.toLoc = toLoc; this.toTime = toTime;
        this.seatList = seatList;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.id);
        dest.writeString(this.fromLoc);
        dest.writeString(this.fromTime);
        dest.writeString(this.toLoc);
        dest.writeString(this.toTime);
        dest.writeList(this.seatList);
    }

    protected Ticket(Parcel in) {
        this.id = in.readString();
        this.fromLoc = in.readString();
        this.fromTime = in.readString();
        this.toLoc = in.readString();
        this.toTime = in.readString();
        this.seatList = new ArrayList<Seat>();
        in.readList(this.seatList, Seat.class.getClassLoader());
    }

    public static final Parcelable.Creator<Ticket> CREATOR = new Parcelable.Creator<Ticket>() {
        @Override
        public Ticket createFromParcel(Parcel source) {
            return new Ticket(source);
        }

        @Override
        public Ticket[] newArray(int size) {
            return new Ticket[size];
        }
    };
}
