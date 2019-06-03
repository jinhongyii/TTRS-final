package com.ttrs.ticketbookingapp.User;

import android.annotation.SuppressLint;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.DialogFragment;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.R;
import com.ttrs.ticketbookingapp.Network.*;

import java.util.Objects;

public class RegisterActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View contentView = inflater.inflate(R.layout.activity_register, null, false);
        DrawerLayout container = findViewById(R.id.activity_base);
        container.addView(contentView, 1);

        Button registerButton = findViewById(R.id.button_register);
        registerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                final String username = ((TextView) findViewById(R.id.register_username)).getText().toString();
                final String password1 = ((TextView) findViewById(R.id.register_password)).getText().toString();
                final String password2 = ((TextView) findViewById(R.id.register_password_2)).getText().toString();
                final String email = ((TextView) findViewById(R.id.register_email)).getText().toString();
                final String phone = ((TextView) findViewById(R.id.register_phone)).getText().toString();

                String prompt = null;
                if (!Objects.equals(password1, password2))
                    prompt = getResources().getString(R.string.password_not_match);
                else if (!Form.isValidUsername(username))
                    prompt = getResources().getString(R.string.invalid_username);
                else if (!Form.isValidPassword(password1))
                    prompt = getResources().getString(R.string.invalid_password);
                else if (!Form.isValidEmail(email))
                    prompt = getResources().getString(R.string.invalid_email);
                else if (!Form.isValidPhone(phone))
                    prompt = getResources().getString(R.string.invalid_phone);

                if (prompt != null) {
                    Toast.makeText(RegisterActivity.this, prompt, Toast.LENGTH_SHORT).show();
                } else {
                    String toSend = String.format(getResources().getString(R.string.register), username, password1, email, phone);
                    (new Client()).send(toSend, new SocketCallbackListener() {
                        @SuppressLint("CommitTransaction")
                        @Override
                        public void onFinish(String result) {
                            Log.d("Register Activity", "Received: " + result);
                            if (Objects.equals(result, "-1")) {
                                Looper.prepare();
                                Toast.makeText(RegisterActivity.this, getResources().getString(R.string.register_failed), Toast.LENGTH_SHORT).show();
                                Looper.loop();
                                return;
                            }
                            DialogFragment t = promptAlert.newAlert(result);
                            t.show(getSupportFragmentManager().beginTransaction(), "dialog");
                            try {
                                Thread.sleep(500);
                            } catch (InterruptedException e) {  }
                            Intent intent = new Intent(RegisterActivity.this, LoginActivity.class);
                            startActivity(intent);
                        }

                        @Override
                        public void onError(Exception e) {
                            Looper.prepare();
                            Toast.makeText(RegisterActivity.this, getResources().getString(R.string.register_failed), Toast.LENGTH_SHORT).show();
                            Looper.loop();
                        }
                    });
                }
            }
        });
    }

    public static class promptAlert extends DialogFragment {
        static promptAlert newAlert(String _msg) {
            promptAlert p = new promptAlert();
            Bundle args = new Bundle();
            args.putString("msg", _msg);
            p.setArguments(args);
            return p;
        }

        @NonNull
        @Override
        public Dialog onCreateDialog(@Nullable Bundle savedInstanceState) {
            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle(R.string.register_prompt_title)
                    .setMessage(getArguments().getString("msg", ""))
                    .setPositiveButton(R.string.positive_button, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            dismiss();
                        }
                    });
            return builder.create();
        }
    }
}
