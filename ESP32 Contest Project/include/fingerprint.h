#include <Arduino.h>
#include <as608.h>
#include <oled.h>

#define WAK 42

unsigned short fingerID[20] = {0};
unsigned char counter = 0;
bool addOK = false;
bool chkOK = false;


/**
  * @brief  添加指纹
  * @param  ID: 指纹ID
  * @retval None
  */
void addFingerprint(unsigned short ID) {
    unsigned char ensure;
    unsigned char state = 0, error = 0;
    int undone = 1;
    //oledPrintText(0, 0, "enter", 8);
    while (undone) {
        switch (state) {

            case 0:     //第一次按手指
                error++;
                delay(200);
                if (digitalRead(WAK) == HIGH) {
                    //oledPrintText(0, 0, "scan 1", 8);
                    ensure = getImage();
                    if (ensure == 0x00) {
                        //oledPrintText(0, 1, "scan 1 ok", 8);
                        ensure = genChara(0x01);    //生成特征
                        if (ensure == 0x00) {
                            //oledPrintText(0, 2, "gen 1 ok", 8);
                            state = 1;
                            error = 0;
                        } else {
                            //oledPrintText(0, 1, "gen ERROR", 8);
                        }
                    }
                }
                break;

            case 1:     //第二次按手指
                error++;
                delay(200);
                if (digitalRead(WAK) == HIGH) {
                    //oledPrintText(64, 0, "scan 2", 8);
                    ensure = getImage();
                    //oledPrintText(64, 1, "scan 2 OK", 8);
                    if (ensure == 0x00) {
                        ensure = genChara(0x02);    //生成特征
                        if (ensure == 0x00) {
                            //oledPrintText(64, 2, "gen 2 ok!!!", 8);
                            state = 2;
                            error = 0;
                        } else {
                            //oledPrintText(64, 1, "gen 2 ERROR", 8);
                        }
                    }
                }
                break;

            case 2:     //特征比对
                error++;
                delay(10);
                ensure = matchChara();
                if (ensure == 0x00) {
                    state = 3;
                    //oledPrintText(0, 3, "STATE2 OK", 8);
                } else {
                    state = 0;
                    error = 0;
                }
                break;        

            case 3:     //生成指纹模板
                delay(10);
                ensure = genTemplate();
                if (ensure == 0x00) {
                    state = 4;
                    //oledPrintText(0, 4, "STATE3 OK", 8);
                } else {
                    state = 0;
                }
                break;

            case 4:     //保存指纹模板
                delay(50);
                ensure = storeTemplate(0x02, ID);
                if (ensure == 0x00) {
                    fingerID[counter] = ID;
                    undone = 0;
                    addOK = true;
                    //oledPrintText(0, 5, "STATE4 OK", 8);
                    break;
                } else {
                    addOK = false;
                    state = 0;
                }
                break;
        }

        delay(250);

        if (error >= 5) {
            error = 0;
            undone = 0;
            addOK = false;
            //oledPrintText(0, 7, "failed to add fingerprint", 8);
            delay(100);
        }
    }
}

unsigned short scanFingerprint() {
    unsigned char i = 0;
    unsigned char ID;
    unsigned char ensure;

            delay(500);
            ensure = getImage();


            if (ensure == 0x00) {
                for (i = 0; i < 12; i++) {
                    rxBuffer[i] = 0x00;                    
                }
                    ensure = genChara(0x01);
                if (ensure == 0x00) {
                    delay(50);
                    for (i = 0; i < 12; i++) {
                        rxBuffer[i] = 0x00;                    
                    }

                    ensure = highSpeedSearch(0x01, &ID, 0, 250);

                    if (ensure == 0x00) {
                        chkOK = true;
                    } else {
                        chkOK = false;
                    }
                }
            }

    return ID;
}