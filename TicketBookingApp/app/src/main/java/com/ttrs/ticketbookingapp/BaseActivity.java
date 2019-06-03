package com.ttrs.ticketbookingapp;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import androidx.core.content.res.ResourcesCompat;
import androidx.core.view.GravityCompat;
import androidx.appcompat.app.ActionBarDrawerToggle;

import android.view.MenuItem;

import com.bumptech.glide.Glide;
import com.bumptech.glide.request.RequestOptions;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;
import com.google.android.material.navigation.NavigationView;

import androidx.drawerlayout.widget.DrawerLayout;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.view.Menu;
import android.view.inputmethod.InputMethodManager;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;

import java.io.File;
import java.util.Objects;

import com.ttrs.ticketbookingapp.Ticket.MyTicket;
import com.ttrs.ticketbookingapp.Ticket.QueryTicket;
import com.ttrs.ticketbookingapp.Train.QueryTrain;
import com.ttrs.ticketbookingapp.User.LoginActivity;
import com.ttrs.ticketbookingapp.User.RegisterActivity;
import com.ttrs.ticketbookingapp.User.UserSetting;

public class BaseActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener {

    private NavigationView navigationView;
    protected ActionBarDrawerToggle toggle;
    protected DrawerLayout drawer;
    protected InterstitialAd mInterstitialAd;
//    protected ProgressBar spinner;


    private void updateDrawerInfo() {
        SharedPreferences userInfo = getSharedPreferences("userInfo", MODE_PRIVATE);
        View header = navigationView.getHeaderView(0);

        String id = userInfo.getString("id", "");
        if (!Objects.equals(id, "")) LoginActivity.updateStatus(getApplicationContext(), id);

        String username = userInfo.getString("username", getResources().getString(R.string.not_logged_in));
        ((TextView) header.findViewById(R.id.userName)).setText(username);
        String email = userInfo.getString("email", "");
        ((TextView) header.findViewById(R.id.userEmail)).setText(email);

        Menu menu = navigationView.getMenu();
        Log.d("BaseActivity", "Username: " + username);
        Boolean logged_in = !Objects.equals(username, getResources().getString(R.string.not_logged_in));
        menu.findItem(R.id.nav_myAccount).setVisible(logged_in);
        menu.findItem(R.id.nav_login).setVisible(!logged_in);
        menu.findItem(R.id.nav_register).setVisible(!logged_in);
        header.setClickable(logged_in);

        File avatar = new File(getFilesDir(), getResources().getString(R.string.avatar));
        if (avatar.exists()) {
            Glide.with(this)
                    .load(avatar)
                    .apply(RequestOptions.circleCropTransform())
                    .into((ImageView) findViewById(R.id.header_userImage));
        } else
            ((ImageView) findViewById(R.id.header_userImage)).setImageDrawable(ResourcesCompat.getDrawable(getResources(), R.drawable.ic_menu_gallery, null));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_base);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        drawer = findViewById(R.id.activity_base);

        navigationView = findViewById(R.id.nav_view);

        //could have used the simpler navigation UI, stupid me!
        toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close) {
            @Override
            public void onDrawerStateChanged(int newState) {
                Log.d("BaseActivity", "State Changed");
                updateDrawerInfo();
            }
        };
        toggle.setDrawerIndicatorEnabled(true);
        drawer.addDrawerListener(toggle);
        toggle.syncState();
        navigationView.setNavigationItemSelectedListener(this);

        View header = navigationView.getHeaderView(0);
        header.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(getApplicationContext(), UserSetting.class);
                startActivity(intent);
            }
        });

        //screws up the original slide-back-on-touch feature of drawerlayout, FUCK!!!
//        findViewById(R.id.activity_base).setOnTouchListener(new View.OnTouchListener() {
//            @Override
//            public boolean onTouch(View v, MotionEvent event) {
//                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
//                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
//                return true;
//            }
//        });

        MobileAds.initialize(this, "ca-app-pub-2844361784519495~4258466834");
        mInterstitialAd = new InterstitialAd(this);
        //real: ca-app-pub-2844361784519495/3300608383
        mInterstitialAd.setAdUnitId("ca-app-pub-3940256099942544/1033173712");
        mInterstitialAd.loadAd(new AdRequest.Builder().build());
        mInterstitialAd.setAdListener(new AdListener() {
            @Override
            public void onAdClosed() {
                AdRequest adRequest = new AdRequest.Builder().addTestDevice("ca-app-pub-3940256099942544/1033173712").build();
                mInterstitialAd.loadAd(adRequest);
            }
        });
    }

    @Override
    public void onBackPressed() {
        DrawerLayout drawer = findViewById(R.id.activity_base);
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        int id = item.getItemId();
        Intent intent;
        switch (id) {
            case R.id.nav_login:
                intent = new Intent(this, LoginActivity.class);
                startActivity(intent);
                break;
            case R.id.nav_register:
                intent = new Intent(this, RegisterActivity.class);
                startActivity(intent);
                break;
            case R.id.nav_logout:
                getSharedPreferences("userInfo", MODE_PRIVATE).edit().clear().apply();
                File avatar = new File(getFilesDir(), getResources().getString(R.string.avatar));
                if (avatar.exists()) avatar.delete();
                updateDrawerInfo();
                intent = new Intent(this, QueryTicket.class);
                startActivity(intent);
                break;
            case R.id.nav_query_train:
                intent = new Intent(this, QueryTrain.class);
                startActivity(intent);
                break;
            case R.id.nav_query_ticket:
                intent = new Intent(this, QueryTicket.class);
                startActivity(intent);
                break;
            case R.id.nav_myTicket:
                intent = new Intent(this, MyTicket.class);
                startActivity(intent);
        }
        DrawerLayout drawer = findViewById(R.id.activity_base);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }
}
