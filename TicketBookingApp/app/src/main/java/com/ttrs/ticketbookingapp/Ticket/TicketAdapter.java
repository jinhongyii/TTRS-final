package com.ttrs.ticketbookingapp.Ticket;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;

import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.ttrs.ticketbookingapp.R;

import java.util.List;

public class TicketAdapter extends RecyclerView.Adapter<TicketAdapter.ViewHolder> {
    static class ViewHolder extends RecyclerView.ViewHolder {
        CardView cardView;
        TextView ticketId, ticketLocTime;

        public ViewHolder(View view, final Context context, final Ticket ticket) {
            super(view);
            cardView = (CardView) view;
            ticketId = cardView.findViewById(R.id.ticketId);
            ticketLocTime = cardView.findViewById(R.id.ticketLocTime);
            view.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent = new Intent(context, TicketInfo.class);
                    intent.putExtra("Ticket", ticket);
                    context.startActivity(intent);
                }
            });
        }
    }

    private List<Ticket> ticketList;
    private Context context;

    public TicketAdapter(List<Ticket> ticketList, Context context) {
        this.ticketList = ticketList;
        this.context = context;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View view = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.ticket_item, viewGroup, false);
        return new TicketAdapter.ViewHolder(view, context, ticketList.get(i));
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder viewHolder, int i) {
        Ticket cur = ticketList.get(i);
        viewHolder.ticketId.setText(cur.id);
        String str = cur.fromLoc + " " + cur.fromTime + " -> " + cur.toLoc + " " + cur.toTime;
        viewHolder.ticketLocTime.setText(str);
    }

    @Override
    public int getItemCount() {
        return ticketList.size();
    }
}
