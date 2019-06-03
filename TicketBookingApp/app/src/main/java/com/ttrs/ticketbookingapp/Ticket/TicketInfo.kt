package com.ttrs.ticketbookingapp.Ticket

import androidx.appcompat.app.ActionBar
import androidx.appcompat.app.AppCompatActivity
import androidx.drawerlayout.widget.DrawerLayout

import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.recyclerview.widget.GridLayoutManager
import com.google.android.material.snackbar.Snackbar

import com.ttrs.ticketbookingapp.BaseActivity
import com.ttrs.ticketbookingapp.Network.Client
import com.ttrs.ticketbookingapp.Network.SocketCallbackListener
import com.ttrs.ticketbookingapp.R
import com.ttrs.ticketbookingapp.User.LoginActivity
import kotlinx.android.synthetic.main.activity_login.*
import kotlinx.android.synthetic.main.activity_ticket_info.*

class TicketInfo : BaseActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val inflater = this.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        val contentView = inflater.inflate(R.layout.activity_ticket_info, null, false)
        val container = findViewById<DrawerLayout>(R.id.activity_base)
        container.addView(contentView, 1)

        val toolbar = supportActionBar
        toggle.isDrawerIndicatorEnabled = false //remove hamburger
        toolbar!!.setDisplayHomeAsUpEnabled(true) //add back button
        toggle.toolbarNavigationClickListener = View.OnClickListener { onBackPressed() }

        val curTicket = intent.getParcelableExtra<Ticket>("Ticket")

        ticketInfo_depart.text = curTicket.fromLoc
        ticketInfo_dest.text = curTicket.toLoc
        ticketInfo_departTime.text = curTicket.fromTime
        ticketInfo_destTime.text = curTicket.toTime

        val isBuy = getSharedPreferences("userInfo", Context.MODE_PRIVATE).getBoolean("isBuy", true)

        for (seat in curTicket.seatList) {
            val button = Button(this)
            button.layoutParams = GridLayout.LayoutParams(
                    GridLayout.spec(GridLayout.UNDEFINED, 1F),
                    GridLayout.spec(GridLayout.UNDEFINED, 1F))
            button.text = if (isBuy) { String.format("%s\n%s元 剩余: %s张", seat.kind, seat.price, seat.numLeft)}
                          else {String.format("%s\n已购买: %s张", seat.kind, seat.numLeft)}

            button.setOnClickListener {
                val builder: AlertDialog.Builder = this.let {
                    AlertDialog.Builder(it)
                }
                val num = EditText(this)
                builder.setTitle(if (isBuy) { R.string.prompt_buy } else { R.string.prompt_refund })
                        .setView(num)
                        .setPositiveButton(R.string.positive_button) { _, _ ->
                            val info = getSharedPreferences("userInfo", Context.MODE_PRIVATE)
                            val id = info.getString("id", "")
                            if (id == "") {
                                val intent = Intent(this, LoginActivity::class.java)
                                startActivity(intent)
                            } else {
                                if (isBuy) {
                                    val toSend = String.format(getString(R.string.buy_ticket), id, num.text, curTicket.id,
                                            curTicket.fromLoc, curTicket.toLoc, curTicket.fromTime.substring(0, 10), seat.kind)
                                    Client().send(toSend, object : SocketCallbackListener {
                                        override fun onFinish(result: String) {
                                            runOnUiThread {
                                            if (result == "1") {
                                                    Snackbar.make(activity_ticket_info, R.string.buy_success, Snackbar.LENGTH_SHORT).show()
                                                    seat.numLeft -= Integer.parseInt(num.text.toString())
                                                    button.text = "${seat.kind}\n${seat.price}元 剩余: ${seat.numLeft}张"
                                                } else Snackbar.make(activity_ticket_info, R.string.buy_fail, Snackbar.LENGTH_SHORT).show()
                                            }
                                        }

                                        override fun onError(e: Exception) {
                                            runOnUiThread {
                                                Snackbar.make(activity_ticket_info, R.string.buy_fail, Snackbar.LENGTH_SHORT).show()
                                            }
                                        }
                                    })
                                } else {
                                    val toSend = String.format(getString(R.string.refund_ticket), id, num.text, curTicket.id,
                                            curTicket.fromLoc, curTicket.toLoc, curTicket.fromTime.substring(0, 10), seat.kind)
                                    Client().send(toSend, object : SocketCallbackListener {
                                        override fun onFinish(result: String) {
                                            runOnUiThread {
                                                Log.d("RefundTicket", result)
                                                if (result == "1") {
                                                    Snackbar.make(activity_ticket_info, R.string.refund_success, Snackbar.LENGTH_SHORT).show()
                                                    seat.numLeft -= Integer.parseInt(num.text.toString())
                                                    button.text = "${seat.kind}\n 已购买: ${seat.numLeft}张"
                                                } else Snackbar.make(activity_ticket_info, R.string.buy_fail, Snackbar.LENGTH_SHORT).show()
                                            }
                                        }

                                        override fun onError(e: Exception) {
                                            runOnUiThread {
                                                Snackbar.make(activity_ticket_info, R.string.buy_fail, Snackbar.LENGTH_SHORT).show()
                                            }
                                        }
                                    })
                                }
                            }
                        }
                        .setNegativeButton(R.string.cancel) { dialog, _ ->
                            dialog.cancel()
                        }
                val dialog = builder.create()
                dialog.show()
            }
            ticketInfo_seats.addView(button)
        }
    }
}
