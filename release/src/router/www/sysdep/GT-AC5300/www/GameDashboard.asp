<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title><#Web_Title#> - Game Dashboard</title>
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="tmmenu.css">
<link rel="stylesheet" type="text/css" href="menu_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<link rel="stylesheet" type="text/css" href="css/rog_cod.css">
<script type="text/javascript" src="js/loader.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/html5kellycolorpicker.min.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<style>
.traffic_bar{
	width: 0%;
	height: 5px;
	border-radius:5px;
}
.traffic_bar_download{
	background-color: #32ADB2;
}
.traffic_bar_upload{
	background-color: #BCBD4D;
}
.transition_style{
	-webkit-transition: all 2s ease-in-out;
	-moz-transition: all 2s ease-in-out;
	-o-transition: all 2s ease-in-out;
	transition: all 2s ease-in-out;
}
.wl_icon{
	display:inline-block;width:48px;height:48px;background:url('images/New_ui/icon_signal.png');
}
.wl0_icon_on{
	background-position: 0px 0px;
}
.wl1_icon_on{
	background-position: 0px 144px;
}
.wl1_1_icon_on{
	background-position: 0px 96px;
}
.wl2_icon_on{
	background-position: 0px 48px;
}
.wl0_icon_off{
	background-position: 48px 0px;
}
.wl1_icon_off{
	background-position: 48px 144px;
}
.wl1_1_icon_off{
	background-position: 48px 96px;
}
.wl2_icon_off{
	background-position: 48px 48px;
}
.wan_state_icon{
	width: 136px;
	height: 136px;
	margin: 5px 0 0 75px;
}
.wan_icon_connect{
	background: url('images/New_ui/wan-connect.png') no-repeat;
}
.wan_icon_disconnect{
	background: url('images/New_ui/wan-disconnect.png') no-repeat;
}
.aura-scheme-icon-enable{
	background: url('images/New_ui/ic-aurasync-all-focus.png') no-repeat;
}
.aura-scheme-icon{
	background: url('images/New_ui/ic-aurasync-all-normal.png') no-repeat;
}
.aura-scheme{
	width: 37px;
	height: 37px;
	cursor: pointer;
}
.aura-scheme-static{
	background-position: 0 0;
}
.aura-scheme-breath{
	background-position: -36px 0;
}
.aura-scheme-flash{
	background-position: -72px 0;
}
.aura-scheme-rainbow{
	background-position: -108px 0;
}
.aura-scheme-commet{
	background-position: -142px 0;
}
.boost-function{
	width: 150px;
	font-size: 16px;
	font-weight: bolder;
	padding-left: 10px;
	background: #111;
	display: flex;
	align-items: center;
	transform: skew(-30deg);
}
.boost-border-odd{
	border-top: 1px solid rgb(145,7,31);
	border-bottom: 1px solid rgb(145,7,31);
}
.boost-border-even{
	border: 1px solid rgb(145,7,31);
}
.boost-key-checked, .boost-function:hover{
	background: rgba(145,7,31, .7);
}
.boost-text{
	transform: skew(30deg);
}
.rog-title{
	font-family: ROG;
	font-size: 26px;
	color:#BFBFBF;
}
</style>
<script>
// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;
var isDemoMode = ('<% nvram_get("demoMode"); %>' == 1) ? true : false;

google.charts.load('current', {'packages':['corechart']});
<% wanlink(); %>
var wanlink_ipaddr = wanlink_ipaddr();
var ddns_enable = '<% nvram_get("ddns_enable_x"); %>';
var ddns_server_x = '<% nvram_get("ddns_server_x");%>';
var ddnsName = decodeURIComponent('<% nvram_char_to_ascii("", "ddns_hostname_x"); %>');

var dataArray = new Array();
var time = new Date();
var timestamp = time.getTime();
var t_year = time.getFullYear();
var t_mon = time.getMonth();
var t_date = time.getDate();
var t_hour = time.getHours();
var t_min = time.getMinutes();
var t_sec = time.getSeconds();
var wl0_radio = '<% nvram_get("wl0_radio"); %>';
var wl1_radio = '<% nvram_get("wl1_radio"); %>';
var wl2_radio = '<% nvram_get("wl2_radio"); %>';

for(i=0;i<30;i++){
	var temp = [];
	temp = [new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec),  0, '', 0, ''];
	dataArray.push(temp);
	timestamp -= 3000;
	time = new Date(timestamp);
	getTime();
}

google.charts.setOnLoadCallback(function(){
	drawChart(dataArray);
});  

function getTime(){
	t_year = time.getFullYear();
	t_mon = time.getMonth();
	t_date = time.getDate();
	t_hour = time.getHours();
	t_min = time.getMinutes();
	t_sec = time.getSeconds();
}

var aurargb = "";
var aura_settings = new Array();
function initial(){
	if(odm_support){
		$(".banner").attr("class", "banner_COD");
		$("#wan_state_icon").attr("class", "cod_connect");
		$("#pingMap").show();
		$("#aura_field").show();
		$("#boostKey_field").show();
	}
	else if(based_modelid == "GT-AX11000"){
		$("#pingMap").show();
		$("#aura_field").show();
		$("#boostKey_field").show();
	}
	else{
		$("#wan_state_icon").attr("class", "wan_state_icon");
	}

	show_menu();
	update_tarffic();
	check_sw_mode();
	check_wireless();
	updateClientsCount();
	
	httpApi.nvramGetAsync({
		data: ["ping_target"],
		success: function(nvram){
			netoolApiDashBoard.start({
				"type": 1, 
				"target": (nvram.ping_target == "") ? "www.google.com" : nvram.ping_target
			});
		}
	})

	setTimeout(check_eula, 100);


	if (ddns_enable == '0' || ddnsName == '' || ddnsName == isMD5DDNSName()) {
		$('#wan_ip_title').html('WAN IP');
		$('#wan_ip_field').html(wanlink_ipaddr);
	}
	else {
		$('#wan_ip_title').html('DDNS');
		$('#wan_ip_field').html(ddnsName);
	}
	aura_settings = document.form.aurargb_val.value.split(',');
	aurargb = "#" + rgbToHex(aura_settings[0]) + rgbToHex(aura_settings[1]) + rgbToHex(aura_settings[2]);
	setColor(aurargb);

	if(aura_settings[3] == "1"){
		$("#_static").removeClass("aura-scheme-icon").addClass("aura-scheme-icon-enable");
	}
	else if(aura_settings[3] == "2"){
		$("#_breath").removeClass("aura-scheme-icon").addClass("aura-scheme-icon-enable");
	}
	else if(aura_settings[3] == "3"){
		$("#_flash").removeClass("aura-scheme-icon").addClass("aura-scheme-icon-enable");
	}
	else if(aura_settings[3] == "6"){
		$("#_rainbow").removeClass("aura-scheme-icon").addClass("aura-scheme-icon-enable");
	}
	else if(aura_settings[3] == "8"){
		$("#_commet").removeClass("aura-scheme-icon").addClass("aura-scheme-icon-enable");
	}

	var ch = eval('<% channel_list_5g(); %>');
	if(isSupport("triband"))
		ch += eval('<% channel_list_5g_2(); %>');
	if(ch.indexOf("52") != -1 || ch.indexOf("56") != -1 || ch.indexOf("60") != -1 || ch.indexOf("64") != -1 || ch.indexOf("100") != -1 || ch.indexOf("104") != -1 || ch.indexOf("108") != -1 || ch.indexOf("112") != -1 || ch.indexOf("116") != -1 || ch.indexOf("120") != -1 || ch.indexOf("124") != -1 || ch.indexOf("128") != -1 || ch.indexOf("132") != -1 || ch.indexOf("136") != -1 || ch.indexOf("140") != -1 || ch.indexOf("144") != -1){
		document.getElementById("boost_dfs").style.display = "";
	}
	var boost_key = '<% nvram_get("turbo_mode"); %>';
	var _array = ["boost_led", "boost_dfs", "boost_aura", "boost_qos"];
	$("#" + _array[boost_key]).addClass("boost-key-checked");
}

function check_eula(){
	ASUS_EULA.config(check_eula, check_eula);

	var asus_status = httpApi.nvramGet(["ASUS_EULA", "ASUS_EULA_time", "ddns_enable_x", "ddns_server_x"], true);
	if( (asus_status.ASUS_EULA == "1" && asus_status.ASUS_EULA_time == "") ||
		(asus_status.ASUS_EULA == "0" && asus_status.ddns_enable_x == "1" && asus_status.ddns_server_x == "WWW.ASUS.COM") ){
		ASUS_EULA.check("asus");
		return false;
	}

	var tm_status = httpApi.nvramGet(["TM_EULA", "TM_EULA_time"], true);
	if(tm_status.TM_EULA == "1" &&  tm_status.TM_EULA_time == ""){
		ASUS_EULA.check("tm");
		return false;
	}
}

function check_sw_mode(){
	var mode = "";
	if(sw_mode == "1")
		mode = "<#wireless_router#>";
	else if(sw_mode == "2"){
		if(wlc_express == 1)
			mode = "<#OP_RE2G_item#>";
		else if(wlc_express == 2)
			mode = "<#OP_RE5G_item#>";
		else
			mode = "<#OP_RE_item#>";
	}
	else if(sw_mode == "3")
		mode = "<#OP_AP_item#>";
	else if(sw_mode == "4")
		mode = "<#OP_MB_item#>";
	else
		mode = "Unknown";	

	$("#sw_mode_desc").html(mode);
}

function check_wireless(){
	var temp = "";
	//check 2.4 GHz
	temp = (wl0_radio == "1") ? "wl0_icon_on" : "wl0_icon_off"
	$("#wl0_icon").addClass(temp);

	//check 5 GHz-1
	if(band5g_support){
		temp = (wl1_radio == "1") ? "wl1_icon_on" : "wl1_icon_off"
		if(band5g2_support){
			temp = (wl1_radio == "1") ? "wl1_1_icon_on" : "wl1_1_icon_off"
		}

		$("#wl1_icon").show();
		$("#wl1_icon").addClass(temp);
	}

	//check 5 GHz-2
	if(band5g2_support){
		temp = (wl2_radio == "1") ? "wl2_icon_on" : "wl2_icon_off"

		$("#wl2_icon").show();
		$("#wl2_icon").addClass(temp);
	}
}

function drawChart(data){
	var dataTable = new google.visualization.DataTable();
	dataTable.addColumn('datetime', 'Date');
	dataTable.addColumn('number', '<#tm_transmission#>');
	dataTable.addColumn({type: 'string', role: 'tooltip'});
	dataTable.addColumn('number', '<#tm_reception#>');
	dataTable.addColumn({type: 'string', role: 'tooltip'});
	dataTable.addRows(data);

	var options = {
		backgroundColor: "transparent",
			title: 'Internet Traffic',
			legend: { position: 'top' },
			legendTextStyle: { color: '#BFBFBF' },
			colors: ['#BCBD4D', '#32ADB2'],
			vAxis: {
				format: '#,### KB/s',
				gridlines: {count: 4},
				textStyle: {color: '#BFBFBF'},
			},
			hAxis: {
				textStyle: {color: '#BFBFBF'}
			}
	}

	var chart = new google.visualization.AreaChart(document.getElementById('area_chart'));   
    chart.draw(dataTable, options);
}
var last_rx = 0
var last_tx = 0
var current_rx = 0;
var current_tx = 0;
var traffic_array = new Array();
function update_tarffic() {
	$.ajax({
    	url: '/update.cgi',
    	dataType: 'script',	
    	data: {'output': 'netdev'},
    	error: function(xhr) {
			setTimeout("update_tarffic();", 1000);
    	},
    	success: function(response){
    		var diff_rx = 0;
    		if(last_rx != 0){
    			if((current_rx - last_rx) < 0){
    				diff_rx = 1;
    			}
    			else{
    				diff_rx = (current_rx - last_rx)/2;
    			}
    		}

    		last_rx = current_rx;
    		if(netdev.INTERNET){
    			current_rx = netdev.INTERNET.rx;
    		}
    		else{
    			current_rx = netdev.WIRED.rx + netdev.WIRELESS0.rx + netdev.WIRELESS1.rx;
    			if( netdev.WIRELESS2){
    				current_rx += netdev.WIRELESS2.rx;
    			}
    		}

    		var diff_tx = 0;
    		if(last_tx != 0){
    			if((current_tx - last_tx) < 0){
    				diff_tx = 1;
    			}
    			else{
    				diff_tx = (current_tx - last_tx)/2;
	    		}
    		}

    		last_tx = current_tx;
    		if(netdev.INTERNET){
    			current_tx = netdev.INTERNET.tx;
    		}
    		else{
    			current_tx = netdev.WIRED.tx + netdev.WIRELESS0.tx + netdev.WIRELESS1.tx;
    			if( netdev.WIRELESS2){
    				current_tx += netdev.WIRELESS2.tx;
    			}
    		}
    		
    		traffic_bar(diff_tx, diff_rx);
    		refineData(diff_tx, diff_rx);
			setTimeout("update_tarffic();drawChart(dataArray);", 2000);
    	}
	});
}

function traffic_bar(tx, rx){
	/*basic unit: Bytes*/
	var temp = "";
	temp = translate_traffic(tx);
	$("#upload_traffic").html(temp[0] + " " + temp[1]);
	$("#upload_traffic_bar").css("width", temp[2]);
	temp = translate_traffic(rx);
	$("#download_traffic").html(temp[0] + " " + temp[1]);
	$("#download_traffic_bar").css("width", temp[2]);
}

function translate_traffic(traffic){
	var value = 0;
	var unit = "Bytes";
	var bar_width = "";
	var bar_value = 0;

	if(traffic != 0)
		bar_value = 1;

	if(traffic > 1024){
		traffic = traffic/1024;
		unit = "KB";
		if(traffic > 1024){
			traffic = traffic/1024;
			unit = "MB";
			bar_value = parseInt(traffic);
			if(bar_value > 100)
				bar_value = 100;
		}
	}

	bar_width = bar_value + "%";
	return [traffic.toFixed(2), unit, bar_width];
}

function refineData(tx, rx){
	time = new Date();
	getTime();
	var tooltip_tx = "";
	var tooltip_rx = "";
	var label_tx = tx + "Bytes/s";
	var label_rx = rx + "Bytes/s";
	rx_temp = rx;
	tx_temp = tx;
	if(tx > 1024){
		label_tx = (tx/1024).toFixed(2) + "KB/s";
		tx = tx/1024;

		if(tx > 1024){
			label_tx = (tx/1024).toFixed(2) + "MB/s";
		}
	}

	if(rx > 1024){
		label_rx = (rx/1024).toFixed(2) + "KB/s";
		rx = rx/1024;

		if(rx > 1024){
			label_rx = (rx/1024).toFixed(2) + "MB/s";
		}
	}	

	tooltip_tx = new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec) + "\n TX: " + label_tx;
	tooltip_rx = new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec) + "\n RX: " + label_rx;
    var temp = [new Date(t_year, t_mon, t_date, t_hour, t_min, t_sec),  tx_temp/1024, tooltip_tx , rx_temp/1024, tooltip_rx];	

    dataArray.push(temp);
    dataArray.shift();
}

var targetData = {}

function initTargetData(){
	var retObj = {};
	retObj["points"] = [];
	retObj["sum"] = 0;
	retObj["avg"] = 0;
	retObj["pingMax"] = 0;
	retObj["pingMin"] = 9999;
	retObj["jitter"] = 0;
	retObj["loss"] = 0;
	retObj["max"] = 0;
	retObj["min"] = 9999;
	retObj["isDoing"] = true;

	retObj["jitters"] = [];
	retObj["jitterSum"] = 0;
	retObj["jitterAvg"] = 0;

	return retObj;
}

var netoolApiDashBoard = {
	start: function(obj){
		if(!targetData[obj.target]){
			targetData[obj.target] = new initTargetData();
		}

		$.getJSON("/netool.cgi", obj)
			.done(function(data){
				if(data.successful != "0") setTimeout(function(){
					netoolApiDashBoard.check(obj, data.successful)
				}, 2000)
			})
	},

	check: function(obj, fileName){
		$.getJSON("/netool.cgi", {"type":0,"target":fileName})
			.done(function(data){
				var thisTarget = targetData[obj.target];
				var pingVal = (data.result[0].ping !== "") ? parseFloat(data.result[0].ping) : 0;
				var jitterVal = (thisTarget.points.length === 0) ? 0 : Math.abs(pingVal - thisTarget.points[thisTarget.points.length-1]).toFixed(1);

				thisTarget.isDoing = (thisTarget.points.length > 110) ? false : thisTarget.isDoing;
				// ping status
				thisTarget.points.push(pingVal);
				thisTarget.pingMax = (thisTarget.pingMax > pingVal) ? thisTarget.pingMax : pingVal;
				thisTarget.pingMin = (thisTarget.pingMin < pingVal) ? thisTarget.pingMin : pingVal;
				thisTarget.sum += pingVal;
				thisTarget.avg = (thisTarget.sum/thisTarget.points.length).toFixed(1);
				thisTarget.jitter = Math.abs(thisTarget.pingMax - thisTarget.pingMin).toFixed(1);
				thisTarget.loss += (parseInt(data.result[0].loss) / 100);

				// jitter status
				thisTarget.jitters.push(jitterVal)
				thisTarget.jitterMax = (thisTarget.jitterMax > jitterVal) ? thisTarget.jitterMax : jitterVal;
				thisTarget.jitterMin = (thisTarget.jitterMin < jitterVal) ? thisTarget.jitterMin : jitterVal;
				thisTarget.jitterSum += parseFloat(jitterVal);
				thisTarget.jitterAvg = (thisTarget.jitterSum/thisTarget.jitters.length).toFixed(1);

				var gap = parseInt(thisTarget.jitter/4) + 2;
				thisTarget.min = parseInt(thisTarget.pingMin/gap)*gap;
				thisTarget.max = thisTarget.min + gap*4;

				if(thisTarget.isDoing){
					netoolApiDashBoard.render(obj.target);
					netoolApiDashBoard.start(obj);
				}
			})
			.fail(function(data){
				setTimeout(function(){
					netoolApiDashBoard.check(obj, fileName);
				}, 500);
			});
	},

	render: function(target){
		var thisTarget = targetData[target];

		// Ping Graph
		var toPosition = function(point){
			return (170-((point-thisTarget.min)/(thisTarget.max-thisTarget.min))*170);
		}

		$(".yAxis")
			.each(function(id){
				$(this).html(thisTarget.min + (thisTarget.max-thisTarget.min)*id/4 + " ms")
			})

		$("#ping_graph")
			.attr("points", function(){
				return thisTarget.points
					.map(function(el, id){return ((id*3) + "," + toPosition(el));})
					.join(" ");
			});

		$("#ping_avg_graph")
			.attr("points", "0," + toPosition(thisTarget.avg) + " 340," + toPosition(thisTarget.avg));

		$("#pingAvg").html(thisTarget.avg + " ms")

		// Jitter Graph
		var toJitterPosition = function(point){
			var graphHeight = (thisTarget.jitter == 0) ? "999" : thisTarget.jitter;
			return (170-(point/graphHeight)*170);
		}

		$(".yAxisJitter")
			.each(function(id){
				$(this).html(Math.abs(thisTarget.jitter*id/4).toFixed(1) + " ms")
			})

		$("#jitter_graph")
			.attr("points", function(){
				return thisTarget.jitters
					.map(function(el, id){return ((id*3) + "," + toJitterPosition(el));})
					.join(" ");
			});

		$("#jitter_avg_graph")
			.attr("points", "0," + toJitterPosition(thisTarget.jitterAvg) + " 340," + toJitterPosition(thisTarget.jitterAvg));

		$("#jitterAvg").html(thisTarget.jitterAvg + " ms")
	},

	reset: function(obj){
		netoolApiDashBoard.stopAll();
		targetData[obj.target] = new initTargetData();
		$("#ping_graph").attr("points", "0,170");
		$("#ping_avg_graph").attr("points", "0,170");
		$("#jitter_graph").attr("points", "0,170");
	},

	stopAll: function(){
		for(var i in targetData){
			targetData[i].isDoing = false;
		}
	}
}
function updateClientsCount() {
	$.ajax({
		url: '/update_networkmapd.asp',
		dataType: 'script', 
		error: function(xhr) {
			setTimeout("updateClientsCount();", 1000);
		},
		success: function(response){
			var re_tune_client_count = function() {
				var count = 0;
				count = fromNetworkmapd_maclist[0].length;
				for(var i in fromNetworkmapd_maclist[0]){
					if (fromNetworkmapd_maclist[0].hasOwnProperty(i)) {
						if(clientList[fromNetworkmapd_maclist[0][i]] != undefined) {
							if(clientList[fromNetworkmapd_maclist[0][i]].amesh_isRe)
								count--;
						}
					}
				}
				return count;
			};

			var client_count = 0;		
			if(amesh_support && (isSwMode("rt") || isSwMode("ap"))){
				client_count = re_tune_client_count();
			}
			else{
				client_count = fromNetworkmapd_maclist[0].length;
			}

			$("#client_count").html(client_count);
			setTimeout("updateClientsCount();", 5000);
		}
	});
}

function rgbToHex(c){
	c = parseInt(c);
	var hex = c.toString(16);
    return hex.length === 1 ? "0" + hex : hex;
}
function setColor(color){
	colorPicker.setColorByHex(color);
	$("#color_pad").css({"background": color});
	$("#color").val(color.toUpperCase());
}

function hexToR(h) {return parseInt((cutHex(h)).substring(0,2),16)}
function hexToG(h) {return parseInt((cutHex(h)).substring(2,4),16)}
function hexToB(h) {return parseInt((cutHex(h)).substring(4,6),16)}
function cutHex(h) {return (h.charAt(0)=="#") ? h.substring(1,7):h}

function inputColor(color){
	setColor(color);
	var r = hexToR(color)
	var g = hexToG(color)
	var b = hexToB(color)
	aura_settings[0] = r;
	aura_settings[1] = g;
	aura_settings[2] = b;
	submitAura();
}

function submitAura(){
	httpApi.nvramSet({
    	"aurargb_val": aura_settings.join(","),
    	"action_mode": "apply",
    	"rc_service": "start_aurargb"
	});
}

function changeRgbMode(obj){
	var _array = ["_static", "_breath", "_flash", "_rainbow", "_commet"];
	for(i=0;i<_array.length;i++){
		$("#" + _array[i]).removeClass("aura-scheme-icon-enable")
					.addClass("aura-scheme-icon");
	}

	var _id = "_" + obj.id;
	$("#" + _id).removeClass("aura-scheme-icon")
				.addClass("aura-scheme-icon-enable");

	if(obj.id == "static"){
		aura_settings[3] = "1";
	}
	else if(obj.id == "breath"){
		aura_settings[3] = "2";
	}
	else if(obj.id == "flash"){
		aura_settings[3] = "3";
	}
	else if(obj.id == "rainbow"){
		aura_settings[3] = "6";
	}
	else if(obj.id == "commet"){
		aura_settings[3] = "8";
	}

	submitAura();
}

function handleBoostKey(obj){
	var _array = ["boost_led", "boost_dfs", "boost_aura", "boost_qos"];
	for(i=0;i<_array.length;i++){
		$("#" + _array[i]).removeClass("boost-key-checked")
	}

	var _id = obj.id;
	$("#" + _id).addClass("boost-key-checked");

	if(obj.id == "boost_dfs"){
		_boost_key = "1";
	}
	else if(obj.id == "boost_aura"){
		_boost_key = "2";
	}
	else if(obj.id == "boost_qos"){
		_boost_key = "3";
	}
	else{		//default, LED (boost_led)
		_boost_key = "0";
	}

	httpApi.nvramSet({
    	"turbo_mode": _boost_key,
    	"action_mode": "apply",
    	"rc_service": "saveNvram"
	});
}

var isMD5DDNSName = function(){
	var macAddr = '<% nvram_get("lan_hwaddr"); %>'.toUpperCase().replace(/:/g, "");
	return "A"+hexMD5(macAddr).toUpperCase()+".asuscomm.com";
}
</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="GameDashBoard.asp">
<input type="hidden" name="next_page" value="GameDashBoard.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="4">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_wrs;restart_firewall">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="aurargb_val" value="<% nvram_get("aurargb_val"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td width="17">&nbsp;</td>		
		<td valign="top" width="202">				
			<div  id="mainMenu"></div>	
			<div  id="subMenu"></div>		
		</td>					
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>	
		<!--===================================Beginning of Main Content===========================================-->		
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0" >
				<tr>
					<td valign="top" >		
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td class="bg_rog" valign="top">
								<div>								
									<div style="width:100%;height:260px;position: relative;">
										<div class="banner"></div>
									</div>
									
									<div style="width:99%;height:200px;margin-top:20px;">
										<div style="display: inline-block;width:240px;vertical-align: top;text-align: center;">			
											<div style="margin: 0 0 0 30px;">
												<div class="rog-title"><#ROG_WIRELESS_STATE#></div>
												<div style="text-align: right;margin:15px 20px 0 0;">
													<div id="wl0_icon" class="wl_icon"></div>
													<div id="wl1_icon" class="wl_icon" style="display:none"></div>
													<div id="wl2_icon" class="wl_icon" style="display:none"></div>
												</div>
											</div>
											<div style="margin: 15px 0 0 10px;text-align: center;">
												<div id="wan_ip_title" class="rog-title">WAN IP</div>
												<div id="wan_ip_field" style="font-size: 18px;margin-top:15px;color:#57BDBA;word-break: break-all"></div>
											</div>
										</div>
										<div style="display: inline-block;width:270px;">
											<div id="internet_title" class="rog-title" style="text-align: center;text-transform: uppercase;"><#statusTitle_Internet#></div>
											<div id="wan_state_icon" class="wan_state_icon"></div>
											<div id="wan_state" style="font-size: 18px;color:#57BDBA;text-transform: uppercase;text-align: center;margin: -15px 0 0 15px;"></div>
										</div>	
										<div style="display: inline-block;width:180px;vertical-align: top;text-align: center;">
											
											<div>
												<div class="rog-title" style="text-transform: uppercase;"><#DSL_Mode#></div>
												<div id="sw_mode_desc" style="font-size: 18px;margin-top:20px;color:#57BDBA"></div>
											</div>
											<div style="margin-top: 40px;text-align:center;color:#BFBFBF;">
												<div class="rog-title" style="text-transform: uppercase;"><#Full_Clients#></div>
												<div style="margin-top:15px;">
													<span id="client_count" style="font-size: 20px;padding:0 10px;color:#57BDBA"></span>
													<span style="font-size: 14px;color:#57BDBA;text-transform: uppercase;"><#Clientlist_Online#></span>
												</div>	
											</div>
										</div>
									</div>

									<div style="width:99%;height:300px;margin: 10px auto;">
										<div>
											<div class="rog-title" style="margin: 60px 20px 0 100px;"><#ROG_Network_Traffic#></div>
										</div>
										<div style="width:100%;">
											<div id="area_chart" style="width: 600px; height: 310px;margin: 0 auto;display:inline-block;"></div>
											<div style="display:inline-block;vertical-align: top;margin:130px 0 0 -50px;width:170px;">
												<div >
													<div>
														<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
															<div id="download_traffic_bar" class="traffic_bar traffic_bar_download transition_style traffic_bar_boost"></div>
														</div>
													</div>
													<div style="font-size:18px;color:#BFBFBF;margin: 5px 0"><#option_download#></div>
													<div id="download_traffic" style="text-align: right;color:#32ADB2;font-size:22px;margin-bottom:5px;"></div>
												</div>
												<div >
													<div>
														<div style="height:5px;padding:2px;background-color:#2A2523;border-radius:10px;width:100px;">
															<div id="upload_traffic_bar" class="traffic_bar traffic_bar_upload transition_style traffic_bar_boost"></div>
														</div>
													</div>
													<div style="font-size:18px;color:#BFBFBF;margin: 5px 0"><#option_upload#></div>
													<div id="upload_traffic" style="text-align: right;color:#BCBD4D;font-size:22px;margin-bottom:5px;"></div>
												</div>
											</div>
										</div>
									</div>

									<div style="width:99%;height:300px;margin: 10px auto;">
										<div class="rog-title" style="margin:45px 120px;position:absolute;width:200px;"><#ROG_Network_PING#></div>
										<div id="svgPingContainer" style="margin:85px 0px 0px 20px;position:absolute;background-color:#221712;">
											<svg width="340px" height="170px">
												<g>
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="0%"   x2="100%" y2="0%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%"  x2="100%" y2="25%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%"  x2="100%" y2="50%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%"  x2="100%" y2="75%" />
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="100%" x2="100%" y2="100%" />
												</g>							
												<g>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="98%">0 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="78%">25 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="55%">50 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="28%">75 ms</text>
													<text class="yAxis" font-family="Verdana" fill="#999" font-size="8" x="0" y="5%">100 ms</text>
												</g>							

												<polyline id="ping_avg_graph" style="fill:none;stroke:#BCCCDC;stroke-width:1;" points="0,250"></polyline>
												<polyline id="ping_graph" style="fill:none;stroke:#57BDBA;stroke-width:2;z-index:9999" points="0,250"></polyline>
											</svg>
										</div>
										<div style="text-align: center;font-family: rog;font-size:20px;margin:255px 40px;position:absolute;width:300px;color:#BFBFBF"><#Average_value#> : <span id="pingAvg" style="font-size:30px">0 ms</span></div>

										<div class="rog-title" style="margin:45px 500px;position:absolute;width:200px;"><#ROG_PING_DEVIATION#></div>
										<div id="svgJitterContainer" style="margin:85px 0px 0px 400px;position:absolute;background-color:#221712;"> 
											<svg width="340px" height="170px">
												<g>
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="0%"   x2="100%" y2="0%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="25%"  x2="100%" y2="25%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="50%"  x2="100%" y2="50%" />
													<line stroke-width="1" stroke-opacity="0.2" stroke="rgb(255,255,255)" x1="0" y1="75%"  x2="100%" y2="75%" />
													<line stroke-width="1" stroke-opacity="1"   stroke="#221712" x1="0" y1="100%" x2="100%" y2="100%" />
												</g>							
												<g>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="98%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="78%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="55%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="28%">0 ms</text>
													<text class="yAxisJitter" font-family="Verdana" fill="#999" font-size="8" x="0" y="5%">0 ms</text>
												</g>							

												<polyline id="jitter_avg_graph" style="fill:none;stroke:#BCCCDC;stroke-width:1;" points="0,250"></polyline>
												<polyline id="jitter_graph" style="fill:none;stroke:#BCBD4D;stroke-width:2;z-index:9999" points="0,250"></polyline>
											</svg>
										</div>
										<div style="text-align: center;font-family: rog;font-size:20px;margin:255px 420px;position:absolute;width:300px;color:#BFBFBF"><#Average_value#> : <span id="jitterAvg" style="font-size:30px">0 ms</span></div>

									</div>									
									</div>

									<div id="pingMap" style="width:380px;height:350px;margin:15px;display:none"></div>
									<script>
										$("#pingMap").load("/cards/pingMap.html");
									</script>
									<div id="aura_field" style="width:345px;height:425px;margin:-360px 0 0 390px;display:none">
										<div style="display:flex;align-items: center;justify-content: space-around;;">
											<div class="rog-title" style="height:65px;">AURA RGB</div>
											<div style="width: 68px;height:68px;margin-top:10px;background: url('./images/New_ui/img-aurasync-logo.png')"></div>
										</div>
										<div style="display:flex;">
											<div style="position: relative;">
											    <canvas id="picker"></canvas><br>
											</div>

											<div style="margin:30px 0 0 30px;width: 101px;height: 160px;">
												<div id="color_pad" style="width: 115px;height:115px;margin: 5px auto 0 auto;border-radius: 50%;position: relative;">
													 <div style="width: 120px;height:120px;background:url('images/New_ui/img-aurasync-rog-logo-mask.png') no-repeat;background-size:100%;position: absolute;top:-1px;left:-1px;"></div>
												</div>

												<input id="color" value="" style="width:90px;height: 30px;margin:10px 0 0 10px;border: 1px solid rgb(145,7,31);background: rgb(62,3,13);color: #FFF;font-size:16px;padding: 0 5px;" onchange="inputColor(this.value);">
											</div>
											<script>
												var colorPicker = new KellyColorPicker({
											        place : 'picker',
											        size : 200,
											        method : 'quad',
											        input: 'color',
											        input_color : false,
											        userEvents : { mouseuph  : function(event, handler, dot) {
											             $("#color_pad").css({"background": handler.getCurColorHex()});
											             $("#color").val(handler.getCurColorHex().toUpperCase());
											             var _rgb = handler.getCurColorRgb();
											             aura_settings[0] = _rgb.r.toString(); 
											             aura_settings[1] = _rgb.g.toString(); 
											             aura_settings[2] = _rgb.b.toString();
											             submitAura();
											        }, change: function(handler){
											        	$("#color_pad").css({"background": handler.getCurColorHex()});
											        }}
											    });
											</script>
										</div>
										
										<div style="margin-top:25px;font-weight: bolder;font-size: 16px;display:flex;align-items: center;">
											<div style="margin-left:15px;">Enable Aura</div>
											<div style="margin-left:30px;">
												<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="radio_aura_enable"></div>
												<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
													<script type="text/javascript">
														$('#radio_aura_enable').iphoneSwitch('<% nvram_get("aurargb_enable"); %>',
															function(){
																httpApi.nvramSet({
																	"aurargb_enable": "1",
																   	"action_mode": "apply",
																   	"rc_service": "start_aurargb"
																});	
															},
															function(){
																httpApi.nvramSet({
																"aurargb_enable": "0",
															    	"action_mode": "apply",
															    	"rc_service": "start_aurargb"
																});	
															}
														);
													</script>
												</div>
											</div>
										</div>
										<div style="display: flex;flex-wrap: wrap;">	
											<div id="static" style="display:flex;align-items: center;margin: 5px 10px;" onclick="changeRgbMode(this);">
												<div id="_static" class="aura-scheme aura-scheme-icon aura-scheme-static"></div>
												<div style="padding-left: 5px;">STATIC</div>
											</div>
											<div id="breath" style="display:flex;align-items: center;margin: 5px 10px;" onclick="changeRgbMode(this);">
												<div id="_breath" class="aura-scheme aura-scheme-icon aura-scheme-breath"></div>
												<div style="padding-left: 5px;">BREATHING</div>
											</div>
											<div id="flash" style="display:flex;align-items: center;margin: 5px 10px;" onclick="changeRgbMode(this);">
												<div id="_flash" class="aura-scheme aura-scheme-icon aura-scheme-flash"></div>
												<div style="padding-left: 5px;">FLASH</div>
											</div>
											<div id="rainbow" style="display:flex;align-items: center;margin: 5px 10px;" onclick="changeRgbMode(this);">
												<div id="_rainbow" class="aura-scheme aura-scheme-icon aura-scheme-rainbow"></div>
												<div style="padding-left: 5px;">COLOR CYCLE</div>
											</div>
											<div id="commet" style="display:flex;align-items: center;margin: 5px 10px;" onclick="changeRgbMode(this);">
												<div id="_commet" class="aura-scheme aura-scheme-icon aura-scheme-commet"></div>
												<div style="padding-left: 5px;">COMET</div>
											</div>
										</div>									
									</div>
									<div id="boostKey_field" style="width:720px;height:340px;margin: 33px 0 20px 15px;display:none">
										<div style="display:flex;align-items: center;justify-content: space-around;">
											<div>
												<div class="rog-title" style="margin-bottom:50px;">Boost Key</div>
												<div style="width:240px;height: 150px;background: url('./images/New_ui/Img-subProd-base.png') no-repeat;background-size: 100%;"></div>
											</div>
											<div style="width:360px;height: 250px;background: url('./images/New_ui/Img-mainProd-base.png') no-repeat;background-size: 100%;"></div>
										</div>
										<div style="display:flex;width:720px;height: 76px;margin-left:20px;">
											<div style="width:30px;height:76px;background: rgb(145,7,31);transform: skew(-30deg);"></div>
											<div id="boost_led" class="boost-function boost-border-odd" onclick="handleBoostKey(this)">
												<div class="boost-text">LED</div>
											</div>
											<div id="boost_dfs" style="display:none;" class="boost-function boost-border-even" onclick="handleBoostKey(this)">
												<div class="boost-text"><#WLANConfig11b_EChannel_dfs#></div>
											</div>
											<div id="boost_aura" class="boost-function boost-border-odd" onclick="handleBoostKey(this)">
												<div class="boost-text">AURA RGB</div>
											</div>
											<div id="boost_qos" class="boost-function boost-border-even" onclick="handleBoostKey(this)">
												<div class="boost-text">Game Boost</div>
											</div>
										</div>
									</div>
								</div>
								</td>
							</tr>
							</tbody>	
						</table>
					</td>         
				</tr>
			</table>				
		<!--===================================Ending of Main Content===========================================-->		
		</td>		
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="footer"></div>
</form>
</body>
</html>
