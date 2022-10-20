#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <huayi-universal/huayi_types.h>
#include <huayi-universal/huayi_events.h>
#include <huayi-universal/huayi_alarm.h>

int main(int argc, char* argv[])
{
    int sk, i, ret;
    char buf[ALT_MAX_PAYLOAD];
    struct huayi_notification *e;
    HUAYI_EVENTS_TYPE eid[1] = {HUAYI_EVENTS_NOTE};

    char strmac[18];

    printf("test-huayi-kalarm started %s\n", argv[1]);
    sk = huayi_events_create();
    ret = huayi_events_add(eid, 1, sk);
    if (ret != EVENTS_SUCC) {
        printf("huayi_events_add failed!\n");
    }

    while(1) {
        printf("waiting for notification...\n");
        ret = huayi_events_recv(sk, -1, buf, ALT_MAX_PAYLOAD);
        if (ret == 0) {
            printf("huayi_msgq_recvandwait timeout\n");
        } else if (ret == -1) {
            printf("huayi_msgq_recvandwait error\n");
        } else {
            printf("notification received:");
            e = (struct huayi_notification*)buf;
            if (e->time > 0) {
                printf("alarm : %d - %s\n", e->notification_id,
                       HUAYI_NOTE_ID_TOSTR(e->notification_id));
                printf("type  : %s(%d)\n",
                       HUAYI_NOTE_TYPE_TOSTR(e->type), e->type);
                printf("reason: %s\n", e->reason);
                printf("status: %lu\n", e->status);
                printf("time  : %lu\n", e->time);

                switch (e->notification_id) {
                case CNOTE_STA_ASSOCIATION:
                    printf("device_id %s\n", e->info.sta_association.device_id);
                    printf("vap_id %d\n", e->info.sta_association.vap_id);
                    A_MACADDR_TOSTR_18(
                        (A_MACADDR*)e->info.sta_association.bssid,
                        strmac, sizeof(strmac));
                    printf("bssid %s\n", strmac);
                    printf("ssid %s\n", e->info.sta_association.ssid);
                    A_MACADDR_TOSTR_18(
                        (A_MACADDR*)e->info.sta_association.client_macaddr,
                        strmac, sizeof(strmac));
                    printf("client %s\n", strmac);
                    printf("status %s(%d)\n",
                           HUAYI_NOTE_ASSOSTAT_TOSTR(e->info.sta_association.status),
                           e->info.sta_association.status);
                    printf("auth_mode %d\n", e->info.sta_association.auth_mode);
                    printf("channel %d\n", e->info.sta_association.channel);
                    printf("rssi %d\n", e->info.sta_association.rssi);
                    printf("qos %d\n", e->info.sta_association.qos);
                    printf("reason_code %d\n", e->info.sta_association.reason_code);
                    printf("reason %s\n", e->info.sta_association.reason);
                    break;
                case CNOTE_RADIO_DOWN:
                    A_MACADDR_TOSTR_18(
                        (A_MACADDR*)e->info.radio_down.ap_macaddr,
                        strmac, sizeof(strmac));
                    printf("ap_macaddr %s\n", strmac);
                    printf("device_id %s\n", e->info.radio_down.device_id);
                    printf("reason %s\n", e->info.radio_down.reason);
                    break;
                case CNOTE_RADIO_DOWN_RECOV:
                    A_MACADDR_TOSTR_18(
                        (A_MACADDR*)e->info.radio_down_recov.ap_macaddr,
                        strmac, sizeof(strmac));
                    printf("ap_macaddr %s\n", strmac);
                    printf("device_id %s\n", e->info.radio_down_recov.device_id);
                    printf("reason %s\n", e->info.radio_down_recov.reason);
                    break;
                case CNOTE_CHANNEL_SCAN_COMPLETED:
                    printf("device_id %s\n",
                           e->info.channel_scan_completed.device_id);
                    break;
                case CNOTE_WIRELESS_INSTRUSION_DETECTED:
                    A_MACADDR_TOSTR_18(
                        (A_MACADDR*)e->info.wireless_instrusion_detected.ap_macaddr,
                        strmac, sizeof(strmac));
                    printf("ap_macaddr %s\n", strmac);
                    printf("device_id %s\n", e->info.wireless_instrusion_detected.device_id);
					A_MACADDR_TOSTR_18(
                        (A_MACADDR*)e->info.wireless_instrusion_detected.attack_mac,
                        strmac, sizeof(strmac));
					printf("bssid is %s\n",e->info.wireless_instrusion_detected.bssid);
					printf("attack_mac is %s\n",strmac);
                    printf("detect_type %d\n", e->info.wireless_instrusion_detected.detect_type);
                    break;
				case CNOTE_BUTTON_PRESS:
					break;
				case CNOTE_BUTTON_RELEASE:
					break;
				case CNOTE_AP_LOCATION_BEGIN:
					break;
				case CNOTE_AP_LOCATION_END:
					break;
                default:
                    break;
                }
            }
        }
    }

    printf("test-huayi-kalarm exiting\n");
    huayi_events_del(eid, 1, sk);
    huayi_events_destroy(sk);
    return 0;
}

