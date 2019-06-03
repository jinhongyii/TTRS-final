package com.ttrs.ticketbookingapp.User;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Looper;

import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.google.android.material.textfield.TextInputLayout;
import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.R;
import com.ttrs.ticketbookingapp.Network.*;
import com.ttrs.ticketbookingapp.Ticket.QueryTicket;

import java.util.Objects;

public class LoginActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View contentView = inflater.inflate(R.layout.activity_login, null, false);
        DrawerLayout container = findViewById(R.id.activity_base);
        container.addView(contentView, 1);


        Button loginButton = findViewById(R.id.button_login);
        loginButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String id = ((TextView) findViewById(R.id.userId)).getText().toString();
                final String password = ((TextView) findViewById(R.id.password)).getText().toString();
                boolean hasError = false;
                if (!Form.isValidId(id)) {
                    ((TextInputLayout) findViewById(R.id.loginText1)).setError(getResources().getString(R.string.invalid_userId));
                    hasError = true;
//                    Toast.makeText(LoginActivity.this, getResources().getString(R.string.invalid_userId), Toast.LENGTH_SHORT).show();
                } else
                    ((TextInputLayout) findViewById(R.id.loginText1)).setError(null);

                if (!Form.isValidPassword(password)) {
                    ((TextInputLayout) findViewById(R.id.loginText2)).setError(getResources().getString(R.string.invalid_password));
                    hasError = true;
//                    Toast.makeText(LoginActivity.this, getResources().getString(R.string.invalid_password), Toast.LENGTH_SHORT).show();
                } else
                    ((TextInputLayout) findViewById(R.id.loginText2)).setError(null);

                if (!hasError) {
                    String toSend = String.format(getResources().getString(R.string.login), id, password);
                    (new Client()).send(toSend, new SocketCallbackListener() {
                        @Override
                        public void onFinish(final String result) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    Log.d("LoginActivity", "Received: " + result);
                                    if (Objects.equals(result, "1")) {
                                        Toast.makeText(LoginActivity.this, getResources().getString(R.string.login_successful), Toast.LENGTH_SHORT).show();
                                    } else {
                                        Toast.makeText(LoginActivity.this, getResources().getString(R.string.login_failed), Toast.LENGTH_SHORT).show();
                                        return;
                                    }
                                    updateStatus(getApplicationContext(), id);
                                    Intent intent = new Intent(LoginActivity.this, QueryTicket.class);
                                    if (mInterstitialAd.isLoaded() && Math.random() < 0.5) {
                                        mInterstitialAd.show();
                                    }
                                    startActivity(intent);
                                }
                            });
                        }

                        @Override
                        public void onError(Exception e) {
                            Looper.prepare();
                            Toast.makeText(LoginActivity.this, getResources().getString(R.string.login_failed), Toast.LENGTH_SHORT).show();
                            Log.e("LoginActivity", "Error occurred in Connection: " + e.toString());
                            Looper.loop();
                        }
                    });
                }
            }
        });
    }

    public static void updateStatus(final Context context, String id) {
        final SharedPreferences.Editor userInfoEditor = context.getSharedPreferences("userInfo", MODE_PRIVATE).edit();
        userInfoEditor.putString("id", id);
        userInfoEditor.apply();
        (new Client()).send(String.format(context.getResources().getString(R.string.query_profile), id), new SocketCallbackListener() {
            @Override
            public void onFinish(String result) {
                String[] p = result.split("[ ]+");
                Log.d("updateStatus", p.toString());
                userInfoEditor.putString("username", p[0]);
                userInfoEditor.putString("email", p[1]);
                userInfoEditor.putString("phone", p[2]);
                userInfoEditor.putString("privilege", p[3]);
                userInfoEditor.apply();
            }

            @Override
            public void onError(Exception e) {
                Log.e("Read_Status", "Failed: " + e.toString());
            }
        });
    }
}
