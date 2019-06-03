//TODO: check form
package com.ttrs.ticketbookingapp.User;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Looper;

import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.Network.Client;
import com.ttrs.ticketbookingapp.Network.SocketCallbackListener;
import com.ttrs.ticketbookingapp.R;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Objects;

public class UserSetting extends BaseActivity {

    private Bitmap bitmapImage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View contentView = inflater.inflate(R.layout.activity_user_setting, null, false);
        DrawerLayout container = findViewById(R.id.activity_base);
        container.addView(contentView, 1);

        ImageView image = findViewById(R.id.userImage);
        image.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //aks for Permission
//                Log.d("UserSetting", "clicked on image");
//                if (ContextCompat.checkSelfPermission(UserSetting.this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
//                        != PackageManager.PERMISSION_GRANTED) {
//                    ActivityCompat.requestPermissions(UserSetting.this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
//                } else {
//                    openAlbum();
//                }
                openAlbum();
            }
        });
        Button button = findViewById(R.id.button_modify);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                saveImage(bitmapImage);
                SharedPreferences userInfo = getSharedPreferences("userInfo", MODE_PRIVATE);
                final String id = userInfo.getString("id", "");
                String username = ((TextView) findViewById(R.id.modify_username)).getText().toString();
                String password = ((TextView) findViewById(R.id.newPassword)).getText().toString();
                String email = ((TextView) findViewById(R.id.modify_email)).getText().toString();
                String phone = ((TextView) findViewById(R.id.modify_phone)).getText().toString();

                (new Client()).send(String.format(getResources().getString(R.string.modify_profile), id, username, password, email, phone),
                        new SocketCallbackListener() {
                            @Override
                            public void onFinish(String result) {
                                if (Objects.equals(result, "1")) {
                                    LoginActivity.updateStatus(getApplicationContext(), id);
                                    Looper.prepare();
                                    Toast.makeText(UserSetting.this, getResources().getString(R.string.modify_success), Toast.LENGTH_SHORT).show();
                                    Looper.loop();
                                }
                                else {
                                    Looper.prepare();
                                    Toast.makeText(UserSetting.this, getResources().getString(R.string.modify_failed), Toast.LENGTH_SHORT).show();
                                    Looper.loop();
                                }
                            }

                            @Override
                            public void onError(Exception e) {
                                Looper.prepare();
                                Toast.makeText(UserSetting.this, getResources().getString(R.string.modify_failed), Toast.LENGTH_SHORT).show();
                                Log.e("UserSetting", "Error occurred in modification: " + e.toString());
                                Looper.loop();
                            }
                        });
            }
        });
        setCurrentProfile();
    }

    private void setCurrentProfile() {
        SharedPreferences userInfo = getSharedPreferences("userInfo", MODE_PRIVATE);
        String username = userInfo.getString("username", getResources().getString(R.string.not_logged_in));
        String email = userInfo.getString("email", "Email");
        String phone = userInfo.getString("phone", "Phone");
        ((TextView) findViewById(R.id.modify_username)).setText(username);
        ((TextView) findViewById(R.id.modify_email)).setText(email);
        ((TextView) findViewById(R.id.modify_phone)).setText(phone);
        if (Objects.equals(username, getResources().getString(R.string.not_logged_in))) {
            findViewById(R.id.button_modify).setVisibility(View.INVISIBLE);
        }
        File avatar = new File(getFilesDir(), getResources().getString(R.string.avatar));
        if (avatar.exists())
            ((ImageView) findViewById(R.id.userImage)).setImageBitmap(BitmapFactory.decodeFile(avatar.getPath()));
        else
            ((ImageView) findViewById(R.id.userImage)).setImageDrawable(getResources().getDrawable(R.drawable.ic_menu_gallery));
    }

    private void openAlbum() {
        Intent intent = new Intent("android.intent.action.GET_CONTENT");
        intent.setType("image/*");
        startActivityForResult(intent, 2);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (resultCode == RESULT_OK && data != null) {
            try {
                Uri image = data.getData();
                InputStream is = getContentResolver().openInputStream(image);
                ImageView imageView = findViewById(R.id.userImage);
                bitmapImage = BitmapFactory.decodeStream(is);
                imageView.setImageBitmap(bitmapImage);
            } catch (Exception e) {
                Log.e("User Setting", "Image problem: " + e.toString());
            }
        }
    }

    private void saveImage(Bitmap bitmap) {
        File image = new File(getFilesDir(), getResources().getString(R.string.avatar));
        try {
            if (image.exists()) image.delete();
            image.createNewFile();
        } catch (IOException e) {
            Log.e("User Setting", "image error: " + e.toString());
        }

        try {
            FileOutputStream os = new FileOutputStream(image);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, os);
        } catch (Exception e) {
            Log.e("User Setting", e.toString());
        }
    }
}
