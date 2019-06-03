package com.ttrs.ticketbookingapp.Train;

import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.ttrs.ticketbookingapp.R;
import com.ttrs.ticketbookingapp.Utility.ItemTouchHelperAdapter;

import java.util.Collections;
import java.util.List;

public class TrainAdapter extends RecyclerView.Adapter<TrainAdapter.ViewHolder> implements ItemTouchHelperAdapter{

    static class ViewHolder extends RecyclerView.ViewHolder {
        CardView cardView;
        TextView stationLoc, stationTime;

        public ViewHolder(View view) {
            super(view);
            cardView = (CardView) view;
            stationLoc = cardView.findViewById(R.id.stationLoc);
            stationTime = cardView.findViewById(R.id.stationTime);
        }
    }

    private List<Station> stationList;

    public TrainAdapter(List<Station> stationList) {
        this.stationList = stationList;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.train_item, viewGroup, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        Station cur = stationList.get(i);
        viewHolder.stationLoc.setText(cur.loc);
        String[] t = cur.time.split("[ ]+");
        viewHolder.stationTime.setText(String.format("到达时间: %s 出发时间: %s 停留时间: %s", t[0], t[1], t[2]));
    }

    @Override
    public int getItemCount() {
        return stationList.size();
    }

    @Override
    public void onItemMove(int fromPos, int toPos) {
        Collections.swap(stationList, fromPos, toPos);
        notifyItemMoved(fromPos, toPos);
    }

    @Override
    public void onItemDelete(int pos) {
        stationList.remove(pos);
        notifyItemRemoved(pos);
    }
}
