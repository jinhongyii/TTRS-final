//TODO: Check inputted time of arrival, start and stopover

package com.ttrs.ticketbookingapp.Train;

import android.app.Dialog;
import android.app.TimePickerDialog;
import android.content.Context;
import android.content.DialogInterface;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.annotation.NonNull;
import androidx.appcompat.app.ActionBar;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AlertDialog;

import android.content.Intent;
import android.os.Bundle;

import androidx.fragment.app.DialogFragment;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.recyclerview.widget.ItemTouchHelper;

import android.text.format.DateFormat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TimePicker;

import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.Network.Client;
import com.ttrs.ticketbookingapp.Network.SocketCallbackListener;
import com.ttrs.ticketbookingapp.R;
import com.ttrs.ticketbookingapp.Utility.myItemTouchHelperCallBack;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

public class ModifyTrain extends BaseActivity {

    public static class TimePickerFragment extends DialogFragment
            implements TimePickerDialog.OnTimeSetListener {

        private EditText editText;

        public TimePickerFragment(EditText editText) {
            super();
            this.editText = editText;
        }

        @NonNull
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            // Use the current time as the default values for the picker
            final Calendar c = Calendar.getInstance();
            int hour = c.get(Calendar.HOUR_OF_DAY);
            int minute = c.get(Calendar.MINUTE);

            // Create a new instance of TimePickerDialog and return it
            return new TimePickerDialog(getActivity(), this, hour, minute,
                    DateFormat.is24HourFormat(getActivity()));
        }

        public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
            editText.setText(String.format("%02d:%02d", hourOfDay, minute));
        }
    }

    private List<Station> stationList;
    private TrainAdapter mAdapter;
    private String str;
    private ArrayList<String> priceList;
    private RecyclerView recyclerView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        final View contentView = inflater.inflate(R.layout.activity_modify_train_2, null, false);
        DrawerLayout container = findViewById(R.id.activity_base);
        container.addView(contentView, 1);

        ActionBar toolbar = getSupportActionBar();
        toggle.setDrawerIndicatorEnabled(false); //remove hamburger
        toolbar.setDisplayHomeAsUpEnabled(true); //add back button
        toggle.setToolbarNavigationClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBackPressed();
            }
        });


        str = getIntent().getExtras().getString("currentTrain", "");
        priceList = getIntent().getExtras().getStringArrayList("priceList");

        stationList = QueryTrain.parseTrain(str);
        RecyclerView recyclerView = findViewById(R.id.stationRecycler_modify);
        GridLayoutManager layoutManager = new GridLayoutManager(ModifyTrain.this, 1);
        recyclerView.setLayoutManager(layoutManager);
        mAdapter = new TrainAdapter(stationList);
        recyclerView.setAdapter(mAdapter);

        //support swipe card to the left and drag card up and down
        ItemTouchHelper.Callback callback = new myItemTouchHelperCallBack(mAdapter);
        ItemTouchHelper touchHelper = new ItemTouchHelper(callback);
        touchHelper.attachToRecyclerView(recyclerView);

        FloatingActionButton newBtn = findViewById(R.id.button_new_station);
        newBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                AlertDialog.Builder dialog = new AlertDialog.Builder(ModifyTrain.this);
                View addStation = getLayoutInflater().inflate(R.layout.add_station, null, false);
                dialog.setTitle(R.string.configure_station).setView(addStation);

                LinearLayout addStation2 = addStation.findViewById(R.id.add_station);

                final EditText stationName = addStation.findViewById(R.id.stationName);

                final EditText timeArrival = addStation.findViewById(R.id.time_arrival);
                timeArrival.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        showTimePickerDialog(timeArrival);
                    }
                });
                final EditText timeStart = addStation.findViewById(R.id.time_start);
                timeStart.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        showTimePickerDialog(timeStart);
                    }
                });
                final EditText timeStopover = addStation.findViewById(R.id.time_stopover);
                timeStopover.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        showTimePickerDialog(timeStopover);
                    }
                });

                final EditText[] t = new EditText[priceList.size()];
                for (int i = 0; i < priceList.size(); i++) {
                    t[i] = new EditText(ModifyTrain.this);
                    t[i].setHint("Price of " + priceList.get(i) + "(￥)");
                    addStation2.addView(t[i]);
                }

                dialog.setPositiveButton(R.string.add_station, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String[] price = new String[priceList.size()];
                        for (int i = 0; i < priceList.size(); i++)
                            price[i] = "￥" + t[i].getText().toString();

                        stationList.add(new Station(stationName.getText().toString(),
                                timeArrival.getText().toString() + " " + timeStart.getText().toString() + " " + timeStopover.getText().toString(), price));

                        mAdapter.notifyDataSetChanged();
                    }
                });
                dialog.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
                dialog.show();
            }
        });

        FloatingActionButton confirmBtn = findViewById(R.id.button_confirm_train);
        confirmBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                String toSend = String.format(
                        getResources().getString(!str.trim().equals("") ? R.string.modify_train : R.string.add_train),
                        encode(stationList));
                Log.d("ModifyTrain", toSend);
                (new Client()).send(toSend, new SocketCallbackListener() {
                    @Override
                    public void onFinish(String result) {
                        if (result.equals("0"))
                            Snackbar.make(v, R.string.modify_train_failed, Snackbar.LENGTH_SHORT).show();
                        else {
                            Snackbar.make(v, R.string.modify_train_success, Snackbar.LENGTH_SHORT).show();
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                            }
                            Intent intent = new Intent(ModifyTrain.this, QueryTrain.class);
                            startActivity(intent);
                        }
                    }

                    @Override
                    public void onError(Exception e) {
                        Snackbar.make(v, R.string.modify_train_failed, Snackbar.LENGTH_SHORT).show();
                    }
                });
            }
        });
    }

    //by default rest of the info is stored in intent
    protected String encode(List<Station> stationList) {
        String trainId = getIntent().getExtras().getString("trainId");
        String trainName = getIntent().getExtras().getString("trainName");
        String trainCatalog = getIntent().getExtras().getString("trainCatalog");

        StringBuilder ret = new StringBuilder();
        ret.append(trainId + " " + trainName + " " + trainCatalog + " " + stationList.size() + " " + priceList.size() + " " + String.join(" ", priceList) + "\n");
        for (Station s : stationList) {
            ret.append(s.loc + " " + s.time + " " + String.join(" ", s.price) + "\n");
        }
        return ret.toString();
    }

    protected void showTimePickerDialog(EditText editText) {
        DialogFragment newFragment = new TimePickerFragment(editText);
        newFragment.show(getSupportFragmentManager(), "timePicker");
    }
}
