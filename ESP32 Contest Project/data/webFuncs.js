/*******************************************************
 * ESP32 Control Platform
 * Copyright (c) 2024 WilliTourt 2944925833@qq.com
*******************************************************/

/*
    This is my
    FIRST
    JavaScript code
*/


//用于控制about内容的显示与隐藏
var aboutbutton = document.querySelector('.aboutbutton');
var aboutcontent = document.querySelector('.aboutcontent');

aboutbutton.addEventListener('click', function() {
    aboutcontent.style.visibility = (aboutcontent.style.visibility === 'visible') ? 'hidden' : 'visible';
    aboutcontent.style.opacity = (aboutcontent.style.opacity === '1') ? '0' : '1';
});

document.addEventListener('click', function(event) {
    if (event.target !== aboutbutton && !aboutcontent.contains(event.target) && aboutcontent.style.visibility === 'visible') {
        aboutcontent.style.opacity = '0';
        aboutcontent.style.visibility = 'hidden';
    }
});

//用于控制提交按钮的显示与隐藏
document.addEventListener('DOMContentLoaded', function() {
    var PWDinput = document.querySelector('#keypadPWD');
    var PWDbutton = document.querySelector('.PWD .button');
    var timerInput = document.querySelector('#timerCommand');
    var timerButton = document.querySelector('.timer .button');
    var alarmHour = document.querySelector('#alarmHour');
    var alarmMinute = document.querySelector('#alarmMinute');
    var alarmButton = document.querySelector('.alarm .button');

    PWDinput.addEventListener('input', function() {
        if (PWDinput.value != '' && PWDinput.value != 'Set access password') {
            PWDbutton.classList.add('active');
        } else {
            PWDbutton.classList.remove('active');
        }
    });

    timerInput.addEventListener('input', function() {
        if (timerInput.value != '' && timerInput.value != 'Enter commands') {
            timerButton.classList.add('active');
        } else {
            timerButton.classList.remove('active');
        }
    });

    alarmHour.addEventListener('input', activeAlarmButton);
    alarmMinute.addEventListener('input', activeAlarmButton);
    function activeAlarmButton() {
        if (alarmHour.value != '' && alarmMinute.value != '' && alarmHour.value != 'Hour' && alarmMinute.value != 'Minute') {
            alarmButton.classList.add('active');
        } else {
            alarmButton.classList.remove('active');
        }
    }
});



function handleLED() {
    console.log('handleLED() called');
    fetch('http://' + window.location.hostname + '/led')
       .then(response => response.text())
       .then(data => {
            const ledStatus = data;
            if (ledStatus == 'on') {
                document.getElementById('ledStatus').textContent = 'On';
                document.getElementById('ledStatus').style.color = 'rgb(124, 187, 43)';
            } else {
                document.getElementById('ledStatus').textContent = 'Off';
                document.getElementById('ledStatus').style.color = 'rgb(213, 81, 81)';
            }
            console.log('handleLED() Done');
       })
        .catch(error => console.error('JS LED state error:', error));
}

function handleDoor() {
    console.log('handleDoor() called');
    fetch('http://' + window.location.hostname + '/door')
       .then(response => response.text())
       .then(data => {
            const doorStatus = data;
            if (doorStatus == 'opened') {
                document.getElementById('doorStatus').textContent = 'Opened';
                document.getElementById('doorStatus').style.color = 'rgb(124, 187, 43)';
            } else if (doorStatus == 'closed') {
                document.getElementById('doorStatus').textContent = 'Closed';
                document.getElementById('doorStatus').style.color = 'rgb(213, 81, 81)';
            }
            console.log('handleDoor() Done');
       })
        .catch(error => console.error('JS Door state error:', error));
}

function handleWindow(value) {
    console.log('windowValue: ' + value);
    fetch('http://' + window.location.hostname + '/window', {
        method: 'POST',
        headers: {
            'Content-Type': 'text/plain'
        },
        body: value
    })
        .then(response => response.text())
        .then(data => {
            document.getElementById('windowStatus').textContent = data;
            console.log('handleWindow() Done');
       })
        .catch(error => console.error('JS window state error:', error));
}



//用于将湿温度数据显示在Device Info的湿温度部分
function fetchDHT() {
    fetch('http://' + window.location.hostname + '/dht')
        .then(response => response.text())
        .then(data => {
            const splitData = data.split(',');
            const tempStr = splitData[0].match(/\d+\.?\d*/);
            const humiStr = splitData[1].match(/\d+\.?\d*/);

            if (tempStr == "0" || humiStr == "0") {
                document.getElementById('dhtTemp').textContent = 'N/A';
                document.getElementById('dhtHumi').textContent = 'N/A';
            } else {
                const temp = parseFloat(tempStr);
                const humi = parseFloat(humiStr);
                document.getElementById('dhtTemp').textContent = temp;
                document.getElementById('dhtHumi').textContent = humi;
            }
            console.log('fetchDHT() Done');
        })
        .catch(error => console.error('JS getting DHT data error:', error));
}
setInterval(fetchDHT, 1000);

//用于NTP时间同步
function syncTime() {
    fetch('http://' + window.location.hostname + '/time')
        .then(response => response.text())
        .then(data => document.getElementById('time').textContent = data)
        .catch(error => console.error('JS NTP time error:', error));
}
setInterval(syncTime, 500);

//用于闹钟计时任务
function alarmTask() {
    console.log('alarmTask() called');
    const hour = document.getElementById('alarmHour').value;
    const minute = document.getElementById('alarmMinute').value;
    const command = hour + ':' + minute + ':00';

    if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59) {
        fetch('http://' + window.location.hostname + '/alarm', {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: command
        })
            .then(response => response.text())
            .then(data => {
                document.getElementById('alarmHour').value = '';
                document.getElementById('alarmMinute').value = '';
                document.querySelector('.alarm .button').classList.remove('active');
                document.getElementById('alarmChk').textContent = 'Alarm set at: ' + command;
                document.getElementById('alarmChk').style.display = 'block';
            })
            .catch(error => {
                console.error('Error setting alarm:', error);
                document.getElementById('alarmChk').textContent = 'Failed to set alarm! Server error.';
                document.getElementById('alarmChk').style.color = 'rgb(246, 154, 16)';
                document.getElementById('alarmChk').style.display = 'block';
            });
    } else {
        document.getElementById('alarmChk').textContent = 'Invalid time!';
        document.getElementById('alarmChk').style.color = 'rgb(213, 81, 81)';
        document.getElementById('alarmChk').style.display = 'block';
    }

    setTimeout(() => {document.getElementById('alarmChk').style.display = 'none';}, 5000);
}

//用于自定义定时任务
function timerTask() {
    console.log('timerTask() called');
    const command = document.getElementById('timerCommand').value;
    const format = /^(led|door|window|guard)-(on|off|toggle|open|close|up|down)-(\d{2})-(\d{2})-(\d{2})$/;
    //正则表达式匹配格式 device-action-hh-mm-ss

    if (!format.test(command)) {
        document.getElementById('timerCmdChk').textContent = 'Invalid command! Please check the format.';
        document.getElementById('timerCmdChk').style.color = 'rgb(213, 81, 81)';
        document.getElementById('timerCmdChk').style.display = 'block';
    } else {
        fetch('http://' + window.location.hostname + '/timer', {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: command
        })
            .then(response => response.text())
            .then(data => {
                document.getElementById('timerCommand').value = ''; //清空input
                document.querySelector('.timer .button').classList.remove('active');
                document.getElementById('timerCmdChk').textContent = 'Successfully set timer task!';
                document.getElementById('timerCmdChk').style.color = 'rgb(124, 187, 43)';
                document.getElementById('timerCmdChk').style.display = 'block';
            })
            .catch(error => {
                console.error('Error setting timer task:', error);
                document.getElementById('timerCmdChk').textContent = 'Failed to set timer task! Server error.';
                document.getElementById('timerCmdChk').style.color = 'rgb(246, 154, 16)';
                document.getElementById('timerCmdChk').style.display = 'block';
            });
    }

    console.log('timerTask() Done');
    setTimeout(() => {document.getElementById('timerCmdChk').style.display = 'none';}, 5000);
}

//用于检查闹钟和定时任务
function fetchANT() {
    fetch('http://' + window.location.hostname + '/aNt')
        .then(response => response.text())
        .then(data => {
            const splitData = data.split(',');
            const alarmTime = splitData[0];
            const timerTask = splitData[1];

            if (alarmTime == 0) {
                document.querySelector('#scheduledAlarm').style.display = 'none';
            } else {
                document.querySelector('#scheduledAlarm').textContent = 'Alarm set at: ' + alarmTime;
                document.querySelector('#scheduledAlarm').style.display = 'block';
            }

            if (timerTask == 0) {
                document.querySelector('#scheduledTask').style.display = 'none';
            } else {
                document.querySelector('#scheduledTask').textContent = 'Scheduled task: ' + timerTask;
                document.querySelector('#scheduledTask').style.display = 'block';
            }
            console.log('chkAlarmAndTimerTask() Done');
        })
        .catch(error => console.error('JS getting aNt error:', error));
}
setInterval(fetchANT, 5000);

//用于获取设备信息
function getDeviceInfo() {
    fetch('http://' + window.location.hostname + '/device')
        .then(response => response.text())
        .then(data => {
            const splitData = data.split(',');
            const ledStatus = splitData[0];
            const doorStatus = splitData[1];
            const windowStatus = splitData[2];
            const guardStatus = splitData[3];
            const dhtTemp = splitData[4].match(/\d+\.?\d*/);
            const dhtHumi = splitData[5].match(/\d+\.?\d*/);
            const auth = splitData[6];

            if (auth == "true") {
                document.getElementById('guardStatus').textContent = 'OK';
                document.getElementById('guardStatus').style.backgroundColor = 'none';
                document.getElementById('guardStatus').style.boxShadow = 'none';
                document.getElementById('guardStatus').style.color = 'rgb(124, 187, 43)';
            } else {
                document.getElementById('guardStatus').textContent = 'UNAUTHORIZED';
                document.getElementById('guardStatus').style.backgroundColor = 'rgb(213, 81, 81)';
                document.getElementById('guardStatus').style.boxShadow = '0px 0px 10px 0px rgba(213, 81, 81, 1)';
                document.getElementById('guardStatus').style.color = 'rgb(250, 250, 250)';
            }

            if (ledStatus == "on") {
                document.querySelector('#ledStatus').textContent = 'On';
                document.getElementById('ledStatus').style.color = 'rgb(124, 187, 43)';
            } else if (ledStatus == "off") {
                document.querySelector('#ledStatus').textContent = 'Off';
                document.getElementById('ledStatus').style.color = 'rgb(213, 81, 81)';
            }

            if (doorStatus == "opened") {
                document.querySelector('#doorStatus').textContent = 'Opened';
                document.getElementById('doorStatus').style.color = 'rgb(124, 187, 43)';
            } else if (doorStatus == "closed") {
                document.querySelector('#doorStatus').textContent = 'Closed';
                document.getElementById('doorStatus').style.color = 'rgb(213, 81, 81)';
            }

            document.querySelector('#windowStatus').textContent = windowStatus;

            if (guardStatus == "on") {
                document.querySelector('#guardStatus').textContent = 'On';
                document.getElementById('guardStatus').style.color = 'rgb(124, 187, 43)';
            } else if (guardStatus == "off") {
                document.querySelector('#guardStatus').textContent = 'Off';
                document.getElementById('guardStatus').style.color = 'rgb(213, 81, 81)';
            }

            if (dhtTemp == "0" || dhtHumi == "0") {
                document.querySelector('#dhtTemp').textContent = 'N/A';
                document.querySelector('#dhtHumi').textContent = 'N/A';
            } else {
                const temp = parseFloat(dhtTemp);
                const humi = parseFloat(dhtHumi);
                document.querySelector('#dhtTemp').textContent = temp;
                document.querySelector('#dhtHumi').textContent = humi;
            }
            
            console.log('getDeviceInfo() Done');
        })
        .catch(error => console.error('JS getting device info error:', error));
}
setInterval(getDeviceInfo, 1000);

//用于设置门禁密码
function setPWD() {
    console.log('tsetPWD() called');
    const command = document.getElementById('keypadPWD').value;
    const format = /^\d{6}$/;

    if (!format.test(command)) {
        document.getElementById('PWDchk').textContent = 'Invalid password! Please enter 6 digits.';
        document.getElementById('PWDchk').style.color = 'rgb(213, 81, 81)';
        document.getElementById('PWDchk').style.display = 'block';
    } else {
        fetch('http://' + window.location.hostname + '/PWD', {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: command
        })
            .then(response => response.text())
            .then(data => {
                document.getElementById('keypadPWD').value = '';
                document.getElementById('keypadPWD').placeholder = 'Current password: ' + command;
                document.querySelector('.PWD .button').classList.remove('active');
                document.getElementById('PWDchk').textContent = 'Successfully set password!';
                document.getElementById('PWDchk').style.color = 'rgb(124, 187, 43)';
                document.getElementById('PWDchk').style.display = 'block';
            })
            .catch(error => {
                console.error('Error setting PWD:', error);
                document.getElementById('PWDchk').textContent = 'Failed to set password! Server error.';
                document.getElementById('PWDchk').style.color = 'rgb(246, 154, 16)';
                document.getElementById('PWDchk').style.display = 'block';
            });
    }

    console.log('setPWD() Done');
    setTimeout(() => {document.getElementById('PWDchk').style.display = 'none';}, 5000);
}





function addFinger() {
    console.log('addFingerprint() called');
    const id = document.getElementById('fID').value;

    fetch('http://' + window.location.hostname + '/addFinger', {
        method: 'POST',
        headers: {
            'Content-Type': 'text/plain'
        },
        body: id
    })
       .then(response => response.text())
       .then(data => {
            console.log("addFR server response: " + data);
            if (data == "ok"){
                document.getElementById('fID').value = '';
                document.getElementById('authChk').textContent = 'Successfully added fingerprint!';
                document.getElementById('authChk').style.color = 'rgb(124, 187, 43)';
                document.getElementById('authChk').style.display = 'block';
                // addFingerToTable(id);
            } else {
                document.getElementById('authChk').textContent = 'Failed to add fingerprint! Try again.';
                document.getElementById('authChk').style.color = 'rgb(213, 81, 81)';
                document.getElementById('authChk').style.display = 'block';
            }
        })
        .catch(error => {
            console.error('Error adding fingerprint:', error);
            document.getElementById('authChk').textContent = 'Failed to add fingerprint! Server error.';
            document.getElementById('authChk').style.color = 'rgb(246, 154, 16)';
            document.getElementById('authChk').style.display = 'block';
        });
    console.log('addFingerprint() Done');
}

function delFinger() {
    console.log('delFingerprint() called');
    // const id = document.getElementById('fID').value;

    document.getElementById('fID').value = '';
    document.getElementById('authChk').style.color = 'rgb(246, 154, 16)';
    document.getElementById('authChk').textContent = 'This feature is not available yet!';
    document.getElementById('authChk').style.display = 'block';

    // fetch('http://' + window.location.hostname + '/delFinger', {
    //     method: 'POST',
    //     headers: {
    //         'Content-Type': 'text/plain'
    //     },
    //     body: id
    // })
    //    .then(response => response.text())
    //    .then(data => {
    //         if (data == "ok"){
    //             document.getElementById('authChk').textContent = 'Successfully deleted fingerprint!';
    //             document.getElementById('authChk').style.color = 'rgb(124, 187, 43)';
    //             document.getElementById('authChk').style.display = 'block';
    //             delFingerFromTable(id);
    //         } else {
    //             document.getElementById('authChk').textContent = 'Failed to delete fingerprint! Try again.';
    //             document.getElementById('authChk').style.color = 'rgb(213, 81, 81)';
    //             document.getElementById('authChk').style.display = 'block';
    //         }
    //     })
    //    .catch(error => {
    //         console.error('Error deleting fingerprint:', error);
    //         document.getElementById('authChk').textContent = 'Failed to delete fingerprint! Server error.';
    //         document.getElementById('authChk').style.color = 'rgb(246, 154, 16)';
    //         document.getElementById('authChk').style.display = 'block';
    //     });
    console.log('delFingerprint() Done');
}