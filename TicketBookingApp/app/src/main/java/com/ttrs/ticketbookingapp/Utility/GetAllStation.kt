package com.ttrs.ticketbookingapp.Utility

import net.sourceforge.pinyin4j.PinyinHelper
import net.sourceforge.pinyin4j.format.HanyuPinyinCaseType
import net.sourceforge.pinyin4j.format.HanyuPinyinOutputFormat
import net.sourceforge.pinyin4j.format.HanyuPinyinToneType
import net.sourceforge.pinyin4j.format.HanyuPinyinVCharType
import java.io.File

fun read(filename: String) : List<String> {
    var l = mutableListOf<String>()
    File(filename).forEachLine {
        if (!it.contains("add_train")) {
            l.add(it.split(" ")[0].trim())
        }
    }
    return l.distinct()
}

fun main() {
    var loc = read("C:\\Users\\Gabriel\\Documents\\DataStructure\\TTRS\\TicketBookingApp\\app\\src\\main\\assets\\all.in")
    var fileContent = ""

    for (i in loc) {
        fileContent += "<item> " + i+"|"
        var result = mutableListOf<String>()

        for (c in i) {
            var format = HanyuPinyinOutputFormat()
            format.setCaseType(HanyuPinyinCaseType.LOWERCASE)
            format.setToneType(HanyuPinyinToneType.WITHOUT_TONE)
            format.setVCharType(HanyuPinyinVCharType.WITH_V)
            var t = PinyinHelper.toHanyuPinyinStringArray(c, format)
            if (t.size == 1) result.add(t[0]);
            else result.add(t[1]);
        }
        fileContent += result.joinToString(separator = "") + " </item>\n"
    }
    print(fileContent)
}