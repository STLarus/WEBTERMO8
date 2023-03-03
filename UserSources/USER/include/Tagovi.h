#ifndef _TAGOVI_H_
#define _TAGOVI_H_

//--------XML tagovi-------------------
extern UINT16 XMLIzlazi(UINT8 *);
extern UINT16 XMLMail(UINT8 *);
extern UINT16 XMLMreza(UINT8 *);
extern UINT16 XMLUlazi(UINT8 *);
extern UINT16 XMLStatusi(UINT8 *);
extern UINT16 XMLSustav(UINT8 *);
extern UINT16 XMLLog(UINT8 *);
extern UINT16 XMLTime(UINT8 *);
extern UINT16 XMLCode(UINT8 *);
extern UINT16 XMLSensor1(UINT8 *);
extern UINT16 XMLSensor2(UINT8 *);
extern UINT16 XMLSensor3(UINT8 *);
extern UINT16 XMLSensor4(UINT8 *);
extern UINT16 XMLSensor5(UINT8 *);
extern UINT16 XMLSensor6(UINT8 *);
extern UINT16 XMLSensor7(UINT8 *);
extern UINT16 XMLSensor8(UINT8 *);
extern UINT16 XMLHome(UINT8 *);
extern UINT16 XMLOnline(UINT8 *);
extern UINT16 XMLDisk0(UINT8 *);
extern UINT16 XMLLogfile(UINT8 *);
extern UINT16 XMLAlarmfile(UINT8 *);
extern UINT16 XMLSms(UINT8 *);
extern UINT16 XMLCredit(UINT8 *);
extern UINT16 XMLSmsstatus(UINT8 *);
extern UINT16 XMLStartCredit(UINT8 *);
extern UINT16 XMLSample(UINT8 *);
extern UINT16 XMLHaccp(UINT8 *);
extern UINT16 XMLPass(UINT8 *);
extern UINT16 XMLBackup(UINT8 *);
extern UINT16 XMLSNTP(UINT8 *);


char* xml_name[] = 
{ 
"izlazi",
"mail",
"mreza",
"ulazi",
"statusi",
"sustav",
"time",
"code",
"sensor1",
"sensor2",
"sensor3",
"sensor4",
"sensor5",
"sensor6",
"sensor7",
"sensor8",
"home",
"online",
"disk0",
"logfile",
"alarmi",
"sms",
"credit",
"smsstatus",
"startcredit",
"sample",
"haccp",
"pass",
"backup",
"sntp",
};

UINT16 (* xml_function[])(UINT8 *)={ 
	XMLIzlazi,
	XMLMail,
	XMLMreza,
	XMLUlazi,
	XMLStatusi,
	XMLSustav,
	XMLTime,
	XMLCode,
	XMLSensor1,	
	XMLSensor2,	
	XMLSensor3,	
	XMLSensor4,
	XMLSensor5,	
	XMLSensor6,	
	XMLSensor7,	
	XMLSensor8,	
	XMLHome,
	XMLOnline,
	XMLDisk0,
	XMLLogfile,
	XMLAlarmfile,
	XMLSms,
	XMLCredit,
	XMLSmsstatus,
   XMLStartCredit,
	  XMLSample,
	  XMLHaccp,
	  XMLPass,
	  XMLBackup,
	  XMLSNTP
};

//--------SVA tagovi-------------------
extern UINT8 SaveInput(UINT8 *,UINT16);
extern UINT8 SaveOutput(UINT8 *,UINT16);
extern UINT8 SaveNet(UINT8 *,UINT16);
extern UINT8 SaveMail(UINT8 *,UINT16);
extern UINT8 SaveTime(UINT8 *,UINT16);
extern UINT8 SaveGraph(UINT8 *,UINT16);
extern UINT8 SaveFeed(UINT8 *,UINT16);
extern UINT8 Sensor1(UINT8 *,UINT16);
extern UINT8 Sensor2(UINT8 *,UINT16);
extern UINT8 Sensor3(UINT8 *,UINT16);
extern UINT8 Sensor4(UINT8 *,UINT16);
extern UINT8 Sensor5(UINT8 *,UINT16);
extern UINT8 Sensor6(UINT8 *,UINT16);
extern UINT8 Sensor7(UINT8 *,UINT16);
extern UINT8 Sensor8(UINT8 *,UINT16);
extern UINT8 SaveMail(UINT8 *,UINT16);
extern UINT8 DelFile0(UINT8 *,UINT16);
extern UINT8 DelFile1(UINT8 *,UINT16);
extern UINT8 TestMail(UINT8 *,UINT16);
extern UINT8 Reset(UINT8 *,UINT16);
extern UINT8 SaveSms(UINT8 *,UINT16);
extern UINT8 TestSms(UINT8 *,UINT16);
extern UINT8 SaveSample(UINT8 *,UINT16);
extern UINT8 Pass(UINT8 *,UINT16);
extern UINT8 SaveBack(UINT8 *,UINT16);
extern UINT8 TestBack(UINT8 *,UINT16);
extern UINT8 SaveSNTP(UINT8 *,UINT16);
extern UINT8 TestSNTP(UINT8 *,UINT16);
char* sva_name[] = 
{ 
	"saveinput",
	"saveoutput",
	"savenet",
	"savemail",
	"savetime",
	"savegraph",
	"savefeed",
	"sensor1",
	"sensor2",
	"sensor3",
	"sensor4",
	"sensor5",
	"sensor6",
	"sensor7",
	"sensor8",
	"mail",
	"delfile0",
	"delfile1",
	"testmail",
	"reset",
	"savesms",
	"testsms",
	"savesample",
	"pass",
	"saveback",
	"testbck",
	"savesntp",
	"testsntp"	
};

UINT8 (* sva_function[])(UINT8 *,UINT16)={ 
	SaveInput,
	SaveOutput,
	SaveNet,
	SaveMail,
	SaveTime,
	SaveGraph,
	SaveFeed,
	Sensor1,
	Sensor2,
	Sensor3,
	Sensor4,
	Sensor5,
	Sensor6,
	Sensor7,
	Sensor8,
	SaveMail,
	DelFile0,
	DelFile1,
	TestMail,
	Reset,
	SaveSms,
	TestSms,
	SaveSample,
	Pass,
	SaveBack,
	TestBack,
	SaveSNTP,
	TestSNTP
};


//--------GVA tagovi-------------------
extern UINT16 LoadTable(char *,char *);

#define GVA_FUNC	1	//broj funkcija u GVA nizu
char* gva_name[] = 
{ 
	"table",
};

UINT16 (* gva_function[])(char *,char *)=
{ 
	LoadTable	
};

#endif