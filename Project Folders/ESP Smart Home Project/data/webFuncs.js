
/*******************************************************
 * ESP32-C3 FH4 Ctrl Platform
 * Copyright (c) 2025 WilliTourt 2944925833@qq.com
*******************************************************/

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
    var timerInput2 = document.querySelector('#timerCommand2');
    var timerButton2 = document.querySelector('.timer2 .button');
    var alarmHour = document.querySelector('#alarmHour');
    var alarmMinute = document.querySelector('#alarmMinute');
    var alarmSecond = document.querySelector('#alarmSecond');
    var alarmButton = document.querySelector('.alarm .button');

    PWDinput.addEventListener('input', function() {
        if (PWDinput.value != '') {
            PWDbutton.classList.add('active');
        } else {
            PWDbutton.classList.remove('active');
        }
    });

    timerInput.addEventListener('input', function() {
        if (timerInput.value != '') {
            timerButton.classList.add('active');
        } else {
            timerButton.classList.remove('active');
        }
    });

    timerInput2.addEventListener('input', function() {
        if (timerInput2.value != '') {
            timerButton2.classList.add('active');
        } else {
            timerButton2.classList.remove('active');
        }
    });

    alarmHour.addEventListener('input', activeAlarmButton);
    alarmMinute.addEventListener('input', activeAlarmButton);
    alarmSecond.addEventListener('input', activeAlarmButton);
    function activeAlarmButton() {
        if (alarmHour.value != '' && alarmMinute.value != '' && alarmSecond.value != '' && alarmHour.value != 'Hour' && alarmMinute.value != 'Min' && alarmSecond.value != 'Sec') {
            alarmButton.classList.add('active');
        } else {
            alarmButton.classList.remove('active');
        }
    }
});



function handleLight(device) {
    console.log('handleLight() called');

    var elementID;
    var route;
    switch (device) {
        case 'bedroom':
            elementID = 'bedroomLight';
            route = '/bedroomLight' 
            break;
        case 'livingroom':
            elementID = 'livingroomLight';
            route = '/livingroomLight'
            break;
        case 'diningroom':
            elementID = 'diningroomLight';
            route = '/diningroomLight'
            break;
    }

    fetch('http://' + window.location.hostname + route)
        .then(response => response.text())
        .then(data => {
            if (data == 'Connection Failed') {
                document.getElementById(elementID).classList.remove('on');
                document.getElementById(elementID).classList.add('error');
                console.error('handleLight() Connection Failed');
            } else {
                const lightStatus = data;
                if (lightStatus == 'on') {
                    document.getElementById(elementID).classList.remove('off');
                    document.getElementById(elementID).classList.add('on');
                } else if (lightStatus == 'off') {
                    document.getElementById(elementID).classList.remove('on');
                    document.getElementById(elementID).classList.add('off');
                }
                console.log('handleLight() Done');
            }
        })
        .catch(error => console.error('JS light state error:', error));
}

function handleDoor() {
    console.log('handleDoor() called');
    fetch('http://' + window.location.hostname + '/door')
       .then(response => response.text())
       .then(data => {
            const doorStatus = data;
            if (doorStatus == 'opened') {
                document.getElementById('doorStatus').style.color = 'rgb(124, 187, 43)';
            } else if (doorStatus == 'closed') {
                document.getElementById('doorStatus').style.color = 'rgb(213, 81, 81)';
            }
            console.log('handleDoor() Done');
       })
        .catch(error => console.error('JS Door state error:', error));
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
    const second = document.getElementById('alarmSecond').value;
    const command = hour + ':' + minute + ':' + second;

    if (hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59) {
        fetch('http://' + window.location.hostname + '/alarm', {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: command
        })
            .then(response => response.text())
            .then(data => {
                if (data == 'Alarm Scheduled') {
                    document.getElementById('alarmHour').value = '';
                    document.getElementById('alarmMinute').value = '';
                    document.getElementById('alarmSecond').value = '';
                    document.querySelector('.alarm .button').classList.remove('active');
                    document.getElementById('alarmChk').textContent = 'Alarm set at: ' + command;
                    document.getElementById('alarmChk').style.display = 'block';
                } else {
                    document.getElementById('alarmChk').textContent = 'Failed to set alarm!';
                    document.getElementById('alarmChk').style.color = 'rgb(213, 81, 81)';
                    document.getElementById('alarmChk').style.display = 'block';
                }
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
    const format = /^(bed|living|dining|bedroomLight|livingroomLight|diningroomLight)-(on|off|open|close)-(\d{2})-(\d{2})-(\d{2})$/;
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
                if (data == 'Timer Scheduled') {
                    document.getElementById('timerCommand').value = ''; //清空input
                    document.querySelector('.timer .button').classList.remove('active');
                    document.getElementById('timerCmdChk').textContent = 'Successfully set timer task!';
                    document.getElementById('timerCmdChk').style.color = 'rgb(124, 187, 43)';
                    document.getElementById('timerCmdChk').style.display = 'block';
                } else {
                    document.getElementById('timerCmdChk').textContent = 'Failed to set timer task! Server error.';
                    document.getElementById('timerCmdChk').style.color = 'rgb(246, 154, 16)';
                    document.getElementById('timerCmdChk').style.display = 'block';
                }
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

function timerTask2() {
    console.log('timerTask2() called');
    const command = document.getElementById('timerCommand2').value;
    const format = /^(bed|living|dining|bedroomLight|livingroomLight|diningroomLight)-(on|off|open|close)-(\d{2})-(\d{2})-(\d{2})$/;
    //正则表达式匹配格式 device-action-hh-mm-ss

    if (!format.test(command)) {
        document.getElementById('timerCmdChk2').textContent = 'Invalid command! Please check the format.';
        document.getElementById('timerCmdChk2').style.color = 'rgb(213, 81, 81)';
        document.getElementById('timerCmdChk2').style.display = 'block';
    } else {
        fetch('http://' + window.location.hostname + '/timer2', {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain'
            },
            body: command
        })
            .then(response => response.text())
            .then(data => {
                if (data == 'Timer 2 Scheduled') {
                    document.getElementById('timerCommand2').value = ''; //清空input
                    document.querySelector('.timer2 .button').classList.remove('active');
                    document.getElementById('timerCmdChk2').textContent = 'Successfully set timer task!';
                    document.getElementById('timerCmdChk2').style.color = 'rgb(124, 187, 43)';
                    document.getElementById('timerCmdChk2').style.display = 'block';
                } else {
                    document.getElementById('timerCmdChk2').textContent = 'Failed to set timer task! Server error.';
                    document.getElementById('timerCmdChk2').style.color = 'rgb(246, 154, 16)';
                    document.getElementById('timerCmdChk2').style.display = 'block';
                }
            })
            .catch(error => {
                console.error('Error setting timer task 2:', error);
                document.getElementById('timerCmdChk2').textContent = 'Failed to set timer task! Server error.';
                document.getElementById('timerCmdChk2').style.color = 'rgb(246, 154, 16)';
                document.getElementById('timerCmdChk2').style.display = 'block';
            });
    }

    console.log('timerTask2() Done');
    setTimeout(() => {document.getElementById('timerCmdChk2').style.display = 'none';}, 5000);
}

//用于检查闹钟和定时任务
function fetchANT() {
    fetch('http://' + window.location.hostname + '/aNt')
        .then(response => response.text())
        .then(data => {
            const splitData = data.split(',');
            const alarmTime = splitData[0];
            const timerTask = splitData[1];
            const timerTask2 = splitData[2];

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

            if (timerTask2 == 0) {
                document.querySelector('#scheduledTask2').style.display = 'none';
            } else {
                document.querySelector('#scheduledTask2').textContent = 'Scheduled task: ' + timerTask2;
                document.querySelector('#scheduledTask2').style.display = 'block';
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
            
            const bedroomLightStatus = splitData[0];
            const livingroomLightStatus = splitData[1];
            const diningroomLightStatus = splitData[2];
            const doorStatus = splitData[3];
            const dhtTemp = splitData[4].match(/\d+\.?\d*/);
            const dhtHumi = splitData[5].match(/\d+\.?\d*/);
            const displayStatus = splitData[6];

            if (bedroomLightStatus == "on") {
                document.getElementById('bedroomLight').classList.remove('off');
                document.getElementById('bedroomLight').classList.add('on');
            } else if (bedroomLightStatus == "off") {
                document.getElementById('bedroomLight').classList.remove('on');
                document.getElementById('bedroomLight').classList.add('off');
            }

            if (livingroomLightStatus == "on") {
                document.getElementById('livingroomLight').classList.remove('off');
                document.getElementById('livingroomLight').classList.add('on');
            } else if (livingroomLightStatus == "off") {
                document.getElementById('livingroomLight').classList.remove('on');
                document.getElementById('livingroomLight').classList.add('off');
            }

            if (diningroomLightStatus == "on") {
                document.getElementById('diningroomLight').classList.remove('off');
                document.getElementById('diningroomLight').classList.add('on');
            } else if (diningroomLightStatus == "off") {
                document.getElementById('diningroomLight').classList.remove('on');
                document.getElementById('diningroomLight').classList.add('off');
            }

            if (doorStatus == "opened") {
                document.getElementById('doorStatus').classList.remove('off');
                document.getElementById('doorStatus').classList.add('on');
            } else if (doorStatus == "closed") {
                document.getElementById('doorStatus').classList.remove('on');
                document.getElementById('doorStatus').classList.add('off');
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

            if (displayStatus == 1) {
                document.querySelector('.displaymode').style.backgroundColor = 'rgb(12, 245, 151)';
            } else if (displayStatus == 0) {
                document.querySelector('.displaymode').style.backgroundColor = 'rgb(243, 90, 90)';
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

function setDisplayMode() {
    console.log('setDisplayMode() called');
    fetch('http://' + window.location.hostname + '/display')
       .then(response => response.text())
       .then(data => {
            if (data == 'on') {
                document.querySelector('.displaymode').style.backgroundColor =' rgb(12, 245, 151)';
            } else if (data == 'off') {
                document.querySelector('.displaymode').style.backgroundColor = 'rgb(243, 90, 90)';
            }
            console.log('setDisplayMode() Done');
       })
        .catch(error => console.error('JS setDisplayMode error:', error));
}
