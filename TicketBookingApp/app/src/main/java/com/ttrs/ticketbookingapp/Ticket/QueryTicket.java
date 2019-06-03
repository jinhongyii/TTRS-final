package com.ttrs.ticketbookingapp.Ticket;

import android.app.AlertDialog;
import android.app.DatePickerDialog;
import android.content.Context;
import android.content.DialogInterface;

import com.google.android.gms.ads.formats.NativeAd;
import com.google.android.material.snackbar.Snackbar;

import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.Animation;
import android.view.animation.LinearInterpolator;
import android.view.animation.RotateAnimation;
import android.view.inputmethod.InputMethodManager;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.ImageView;

import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.Network.Client;
import com.ttrs.ticketbookingapp.Network.SocketCallbackListener;
import com.ttrs.ticketbookingapp.R;
import com.ttrs.ticketbookingapp.Utility.LocationAutoCompleteAdapter;
import com.ttrs.ticketbookingapp.Utility.Location;

import java.util.ArrayList;
import java.util.List;

public class QueryTicket extends BaseActivity {

    private List<String> catalogList = new ArrayList<>();
    private boolean[] checkedItems = new boolean[26];
    private List<Ticket> ticketList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View contentView = inflater.inflate(R.layout.activity_query_ticket, null, false);
        DrawerLayout container = findViewById(R.id.activity_base);
        container.addView(contentView, 1);

        final AutoCompleteTextView loc1 = findViewById(R.id.query_loc1);
        final AutoCompleteTextView loc2 = findViewById(R.id.query_loc2);
        List<Location> list = new ArrayList<>();
        String[] location = getResources().getStringArray(R.array.location);
        for (String i : location) {
            list.add(new Location(i.split("[|]")[1], i.split("[|]")[0]));
        }

        loc1.setAdapter(new LocationAutoCompleteAdapter(this, android.R.layout.simple_dropdown_item_1line, list));
        loc2.setAdapter(new LocationAutoCompleteAdapter(this, android.R.layout.simple_dropdown_item_1line, list));

        final ImageView swipe = findViewById(R.id.swipeImage);
        final RotateAnimation animation = new RotateAnimation(0, 180, Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
        animation.setInterpolator(new AccelerateDecelerateInterpolator());
        animation.setDuration(700);
        animation.setFillAfter(true);


        swipe.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String t = loc1.getText().toString();
                loc1.setText(loc2.getText().toString());
                loc2.setText(t);
                swipe.startAnimation(animation);
            }
        });

        final EditText date = findViewById(R.id.query_date);
        date.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                DatePickerDialog datePickerDialog = new DatePickerDialog(QueryTicket.this);
                datePickerDialog.setOnDateSetListener(new DatePickerDialog.OnDateSetListener() {
                    @Override
                    public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
                        String formattedMonth = String.format("%02d", month + 1);
                        String formattedDay = String.format("%02d", dayOfMonth);
                        date.setText(year + "-" + formattedMonth + "-" + formattedDay);
                    }
                });
                datePickerDialog.show();
            }
        });

        final EditText catalog = findViewById(R.id.query_catalog);
        catalog.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                AlertDialog.Builder builder = new AlertDialog.Builder(QueryTicket.this);
                builder.setTitle(R.string.prompt_catalog);
                builder.setMultiChoiceItems(R.array.catalog, checkedItems, new DialogInterface.OnMultiChoiceClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which, boolean isChecked) {
                        String c = Character.toString((char) (which + 65));
                        if (isChecked) {
                            catalogList.add(c);
                            checkedItems[which] = true;
                        } else {
                            catalogList.remove(c);
                            checkedItems[which] = false;
                        }
                        Log.d("QueryTicket", catalogList.toString());
                    }
                });
                builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        catalog.setText(String.join("", catalogList));
                    }
                });
                builder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
                builder.show();
            }
        });

        Button searchBtn = findViewById(R.id.button_search_ticket);
        searchBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(v.getWindowToken(), 0);

                String loc1 = ((EditText) findViewById(R.id.query_loc1)).getText().toString();
                String loc2 = ((EditText) findViewById(R.id.query_loc2)).getText().toString();
                String dateStr = date.getText().toString();
                String catStr = catalog.getText().toString();
                boolean isTransfer = ((CheckBox) findViewById(R.id.isTransfer)).isChecked();
                String toSend = String.format(getResources().getString(isTransfer ? R.string.query_transfer_ticket : R.string.query_ticket), loc1, loc2, dateStr, catStr);


                (new Client()).send(toSend, new SocketCallbackListener() {
                    @Override
                    public void onFinish(final String result) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                getSharedPreferences("userInfo", Context.MODE_PRIVATE).edit().putBoolean("isBuy", true).apply();
                                if (mInterstitialAd.isLoaded() && Math.random() < 0.5) {
                                    mInterstitialAd.show();
                                }
                                if (result.equals("-1") || result.equals("0"))
                                    Snackbar.make(v, R.string.ticket_not_found, Snackbar.LENGTH_LONG).show();
                                else {
                                    ticketList = parseTicket(result);
                                    RecyclerView recyclerView = findViewById(R.id.ticketRecycler);
                                    GridLayoutManager layoutManager = new GridLayoutManager(QueryTicket.this, 1);
                                    recyclerView.setLayoutManager(layoutManager);
                                    recyclerView.setAdapter(new TicketAdapter(ticketList, QueryTicket.this));
                                }
                            }
                        });
                    }

                    @Override
                    public void onError(Exception e) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                getSharedPreferences("userInfo", Context.MODE_PRIVATE).edit().putBoolean("isBuy", true).apply();
                                Snackbar.make(v, R.string.query_train_failed, Snackbar.LENGTH_LONG).show();
                                if (mInterstitialAd.isLoaded()) {
                                    mInterstitialAd.show();
                                }
                            }
                        });
                    }
                });
            }
        });
    }

    public static List<Ticket> parseTicket(String str) {
        List<Ticket> ret = new ArrayList<>();
        String[] p = str.split("\n");
        if (str.length() <= 10) return ret;

        for (String cur : p) {
            String[] t = cur.split("[ ]+");
            if (t.length <= 6) continue;
            List<Seat> seats = new ArrayList<>();
            int cnt = 7;
            for (int i = 0; i < (t.length - 7) / 3; i++, cnt += 3) {
                seats.add(new Seat(t[cnt], t[cnt + 2], Integer.parseInt(t[cnt + 1])));
            }
            Ticket curTicket = new Ticket(t[0], t[1], t[2] + " " + t[3], t[4], t[5] + " " + t[6], seats);
            ret.add(curTicket);
        }

        return ret;
    }
}
