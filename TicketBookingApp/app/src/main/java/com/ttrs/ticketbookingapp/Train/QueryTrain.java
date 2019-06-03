package com.ttrs.ticketbookingapp.Train;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Looper;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.text.Layout;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.Network.Client;
import com.ttrs.ticketbookingapp.Network.SocketCallbackListener;
import com.ttrs.ticketbookingapp.R;

import java.util.ArrayList;
import java.util.List;

public class QueryTrain extends BaseActivity {

    private FloatingActionButton saleBtn, modifyBtn, deleteBtn, newBtn;
    private String str;
    private ProgressBar bar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        ConstraintLayout contentView = (ConstraintLayout) inflater.inflate(R.layout.activity_query_train, null, false);
        DrawerLayout container = findViewById(R.id.activity_base);
        container.addView(contentView, 1);

        saleBtn = findViewById(R.id.button_sale);
        saleBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                String trainId = ((TextView) findViewById(R.id.searchText)).getText().toString();
                String toSend = String.format(getResources().getString(R.string.sale_train), trainId);
                new Client().send(toSend, new SocketCallbackListener() {
                    @Override
                    public void onFinish(String result) {
                        if (result.equals("0"))
                            Snackbar.make(v, R.string.sale_failed, Snackbar.LENGTH_SHORT).show();
                        else
                            Snackbar.make(v, R.string.sale_success, Snackbar.LENGTH_SHORT).show();
                    }

                    @Override
                    public void onError(Exception e) {
                        Snackbar.make(v, R.string.sale_failed, Snackbar.LENGTH_SHORT).show();
                    }
                });
            }
        });

        deleteBtn = findViewById(R.id.button_delete_train);
        deleteBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                String trainId = ((TextView) findViewById(R.id.searchText)).getText().toString();
                String toSend = String.format(getResources().getString(R.string.delete_train), trainId);
                ((new Client())).send(toSend, new SocketCallbackListener() {
                    @Override
                    public void onFinish(String result) {
                        if (result.equals("0"))
                            Snackbar.make(v, R.string.delete_success, Snackbar.LENGTH_SHORT).show();
                        else
                            Snackbar.make(v, R.string.delete_success, Snackbar.LENGTH_SHORT).show();
                    }

                    @Override
                    public void onError(Exception e) {
                        Snackbar.make(v, R.string.delete_failed, Snackbar.LENGTH_SHORT).show();
                    }
                });
            }
        });


        modifyBtn = findViewById(R.id.button_modify_train);
        modifyBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(QueryTrain.this, ModifyTrainPre.class);
                intent.putExtra("currentTrain", str);
                intent.putExtra("isNew", false);
                startActivity(intent);
            }
        });

        newBtn = findViewById(R.id.button_new_train);
        newBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(QueryTrain.this, ModifyTrainPre.class);
                intent.putExtra("currentTrain", str);
                intent.putExtra("isNew", true);
                startActivity(intent);
            }
        });

        setVisibility(false);

        bar = findViewById(R.id.progress_circular);
        bar.setVisibility(View.GONE);

        Button searchBtn = findViewById(R.id.searchButton);
        searchBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                bar.setVisibility(View.VISIBLE);

                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                String trainId = ((TextView) findViewById(R.id.searchText)).getText().toString();
                ((new Client())).send(String.format(getResources().getString(R.string.query_train), trainId), new SocketCallbackListener() {
                    @Override
                    public void onFinish(final String result) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                bar.setVisibility(View.GONE);
                                if (mInterstitialAd.isLoaded()) {
                                    Log.d("AD", String.valueOf(Math.random()));
                                    if (Math.random() < 0.5) mInterstitialAd.show();
                                }
                                if (result.equals("0")) {
                                    Toast.makeText(QueryTrain.this, R.string.search_failed, Toast.LENGTH_LONG).show();
                                    setVisibility(false);
                                } else {
                                    setVisibility(true);
                                }
                                str = result;
                                List<Station> stationList = parseTrain(result);
                                RecyclerView recyclerView = findViewById(R.id.stationRecycler);
                                GridLayoutManager layoutManager = new GridLayoutManager(QueryTrain.this, 1);
                                recyclerView.setLayoutManager(layoutManager);
                                recyclerView.setAdapter(new TrainAdapter(stationList));
                            }
                        });
                    }

                    @Override
                    public void onError(Exception e) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if (mInterstitialAd.isLoaded() && Math.random() < 0.5) {
                                    mInterstitialAd.show();
                                }
                                bar.setVisibility(View.GONE);
                                setVisibility(false);
                                Toast.makeText(QueryTrain.this, R.string.search_failed, Toast.LENGTH_LONG).show();
                            }
                        });
                    }
                });
            }
        });
    }

    private void setVisibility(boolean v) {
        String p = getSharedPreferences("userInfo", MODE_PRIVATE).getString("privilege","0");
        if (!p.equals("2")) {
            saleBtn.hide();
            modifyBtn.hide();
            deleteBtn.hide();
            newBtn.hide();
            return;
        }
        if (v) {
            modifyBtn.show();
            newBtn.hide();
        } else {
            modifyBtn.hide();
            newBtn.show();
        }
    }

    public static List<Station> parseTrain(String str) {
        List<Station> ret = new ArrayList<>();
        String[] p = str.split("[ ]+");
        if (p.length <= 5) return ret;

        int step = 4 + Integer.parseInt(p[4]);
        int cnt = step + 1;
        int pt = Integer.parseInt(p[4]);

        for (int i = 0; i < Integer.parseInt(p[3]); i++) {
            Log.d("QueryTrain", p[cnt] + " " + p[cnt + 1] + " " + p[cnt + 2] + " " + p[cnt + 3]);
            String loc, time;

//            if (i == 0) loc = "起点站: " + p[cnt];
//            else loc = p[cnt - step] + " -> " + p[cnt];
            loc = p[cnt];

            String[] price = new String[pt];
            for (int j = 0; j < pt; j++) price[j] = p[cnt + 4 + j];
            Station cur = new Station(loc, p[cnt + 1] + " " + p[cnt + 2] + " " + p[cnt + 3], price);
            ret.add(cur);
            cnt += step;
        }
        return ret;
    }
}
