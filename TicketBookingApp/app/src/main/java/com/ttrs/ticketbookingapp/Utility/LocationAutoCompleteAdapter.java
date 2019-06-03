package com.ttrs.ticketbookingapp.Utility;

import android.content.Context;
import android.text.Layout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Filter;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.List;

public class LocationAutoCompleteAdapter extends ArrayAdapter<Location> {
    private LayoutInflater layoutInflater;
    private List<Location> locationList;

    private Filter filter = new Filter() {
        @Override
        public CharSequence convertResultToString(Object resultValue) {
//            Log.d("AutoComplete", ((Location) resultValue).loc);
            return ((Location) resultValue).loc;
        }

        @Override
        protected FilterResults performFiltering(CharSequence constraint) {
            FilterResults results = new FilterResults();
            List<Location> ret = new ArrayList<>();
            for (Location loc : locationList) {
                if (loc.pinyin.contains(constraint.toString()) || loc.loc.contains(constraint.toString()))
                    ret.add(loc);
            }
            results.values = ret;
            results.count = ret.size();
            return results;
        }

        @Override
        protected void publishResults(CharSequence constraint, FilterResults results) {
            clear();
            if (results != null && results.count > 0) {
                addAll((ArrayList<Location>) (results.values));
            } else {
                Log.d("AutoComplete", locationList.toString());
                addAll(locationList);
            }
            notifyDataSetChanged();
        }
    };

    public LocationAutoCompleteAdapter(Context context, int rid, List<Location> locationList) {
        super(context, rid, locationList);
        Log.d("AutoComplete", locationList.toString());
        this.locationList = new ArrayList<>();
        this.locationList.addAll(locationList);
        layoutInflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @NonNull
    @Override
    public Filter getFilter() {
        return filter;
    }
}
