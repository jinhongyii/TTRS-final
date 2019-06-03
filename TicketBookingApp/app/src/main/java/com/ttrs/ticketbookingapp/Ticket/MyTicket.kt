package com.ttrs.ticketbookingapp.Ticket

import android.content.Context
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.LayoutInflater
import androidx.drawerlayout.widget.DrawerLayout
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.snackbar.Snackbar
import com.ttrs.ticketbookingapp.BaseActivity
import com.ttrs.ticketbookingapp.Network.Client
import com.ttrs.ticketbookingapp.Network.SocketCallbackListener
import com.ttrs.ticketbookingapp.R
import kotlinx.android.synthetic.main.activity_my_ticket.*
import java.lang.Exception
import com.ttrs.ticketbookingapp.Ticket.QueryTicket.*

class MyTicket : BaseActivity() {
    private lateinit var ticketList: List<Ticket>

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val inflater = this.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        val contentView = inflater.inflate(R.layout.activity_my_ticket, null, false)
        val container = findViewById<DrawerLayout>(R.id.activity_base)
        container.addView(contentView, 1)

        val id = getSharedPreferences("userInfo", Context.MODE_PRIVATE).getString("id", "")
        val toSend = String.format(getString(R.string.query_all_ticket), id)
        Client().send(toSend, object : SocketCallbackListener {
            override fun onFinish(result: String?) {
                runOnUiThread {
//                    val result2 = "1\n" +
//                            "c100 北京 2018-03-28 08:00 上海 2018-03-28 08:01 一等座 2000 765.50 二等座 2000 765.49 三\n" +
//                            "等座 2000 765.48"
                    ticketList = parseTicket(result)
                    if (ticketList.isEmpty()) {
                        Snackbar.make(myTicket, R.string.no_ticket, Snackbar.LENGTH_SHORT).show()
                    } else {
                        val viewManager = LinearLayoutManager(this@MyTicket)
                        val viewAdapter = TicketAdapter(ticketList, this@MyTicket)
                        findViewById<RecyclerView>(R.id.myTicketRecycler).apply {
                            layoutManager = viewManager
                            adapter = viewAdapter
                        }
                    }
                }
            }

            override fun onError(e: Exception?) {
                Snackbar.make(myTicket, R.string.error, Snackbar.LENGTH_SHORT).show()
            }
        })


    }
}
