package com.ttrs.ticketbookingapp.Train;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;

import com.google.android.material.floatingactionbutton.FloatingActionButton;

import androidx.appcompat.app.ActionBarDrawerToggle;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toolbar;

import com.ttrs.ticketbookingapp.BaseActivity;
import com.ttrs.ticketbookingapp.R;

import java.util.ArrayList;

public class ModifyTrainPre extends BaseActivity {

    private ArrayList<String> priceList;
    private ArrayAdapter<String> mAdapter;
    private String cur;
    private Boolean isNew;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LayoutInflater inflater = (LayoutInflater) this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        final View contentView = inflater.inflate(R.layout.activity_modify_train_1, null, false);
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

        try {
            cur = getIntent().getExtras().getString("currentTrain", "");
            isNew = getIntent().getBooleanExtra("isNew", false);
        } catch (NullPointerException e) {
            cur = "";
            isNew = false;
        }

        Log.d("ModifyTrainPre", cur);


        final EditText trainId = findViewById(R.id.trainId);
        trainId.setText(getTrain(cur, 0));
        final EditText trainName = findViewById(R.id.trainName);
        trainName.setText(getTrain(cur, 1));
        final EditText trainCatalog = findViewById(R.id.trainCatalog);
        trainCatalog.setText(getTrain(cur, 2));

        priceList = parsePrice(cur);

        mAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, android.R.id.text1, priceList);
        ListView listView = findViewById(R.id.priceList);
        listView.setAdapter(mAdapter);

        FloatingActionButton addBtn = findViewById(R.id.button_new_price);
        addBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                AlertDialog.Builder dialog = new AlertDialog.Builder(ModifyTrainPre.this);
                final EditText price = new EditText(ModifyTrainPre.this);
                dialog.setTitle(R.string.prompt_seat).setView(price);
                dialog.setPositiveButton(R.string.add_seat, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String cur = price.getText().toString();
                        priceList.add(cur);
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

        FloatingActionButton toNextBtn = findViewById(R.id.button_modify_next);
        toNextBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(ModifyTrainPre.this, ModifyTrain.class);
                intent.putExtra("isNew", isNew);
                intent.putExtra("currentTrain", cur);
                intent.putExtra("trainId", trainId.getText().toString());
                intent.putExtra("trainName", trainName.getText().toString());
                intent.putExtra("trainCatalog", trainCatalog.getText().toString());
                intent.putStringArrayListExtra("priceList", priceList);

                Log.d("ModifyTrainPre", trainId.getText().toString() + " " + trainName.getText().toString() + " " + trainCatalog.getText().toString());
                startActivity(intent);
            }
        });
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.d("ModifyTrainPre", "AAA");

        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    protected ArrayList<String> parsePrice(String str) {
        ArrayList<String> ret = new ArrayList<>();
        String[] p = str.split("[ ]+");
        if (p.length <= 5) return ret;

        int num = Integer.parseInt(p[4]);
        for (int i = 1; i <= num; i++) ret.add(p[i + 4]);
        return ret;
    }

    protected String getTrain(String str, int index) {
        String[] p = str.split("[ ]+");
        if (p.length <= 5) return "";
        return p[index];
    }
}
