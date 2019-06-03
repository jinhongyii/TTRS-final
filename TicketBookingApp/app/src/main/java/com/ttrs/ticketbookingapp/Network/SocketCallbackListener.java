package com.ttrs.ticketbookingapp.Network;

public interface SocketCallbackListener {
    void onFinish(String result);
    void onError(Exception e);
}