let command =
  `create --server=http://apis-newfat.hz.mindlinker.cn --bff=http://pub-newfat.hz.mindlinker.cn --callee= --room= --name=13170889099 --nick=13170889099 --avatar= --mobile=13170889099 --im=mc.test.seewo.com --imType=0 --imApp=c836826e3dd0417eb91d21195d7f937d --mic --camera --cloudRecord --localRecord --log=C:\\Users\\user\\AppData\\Roaming\\MindLinkerTest\\MLMeeting --fridayAppId=c3f4f1000182524086bd3f2ae01529a9 --version=5.10.0 --companyId=f3efce85-034e-45c4-9c8d-4078374b76fd --userId=dea059fd-b4f7-404f-bf44-d9945fa95abd --live --watermark --mark --appointmentSite= --appointmentTime= --appointmentBeginTime= --appointmentEndTime= --remoteControl --signable --token=7d157d69-87a2-4318-babe-84a9bd8c5418`.split(
    ' '
  );

module.exports = command;
