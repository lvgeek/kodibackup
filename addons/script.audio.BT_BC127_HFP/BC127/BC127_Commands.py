# BC127 UART Commands

ANSWER [link_ID]
CALL [link_ID] (number)
CLOSE (link_ID) 
CONFIG
DISCOVERABLE (mode) 
END [link_ID] 
GET (config_name) 
LIST
MUSIC [link_ID] (instruction)
NAME (BT_addr) 
OPEN (BT_addr) (profile)
POWER (mode) 
PULL_PBOOK (pbook) 
PULL_ABORT
REJECT 
RESET
SET (config)=value 
STATUS
TOGGLE_VR [link_ID] S
WRITE



# BC127 NOTIFICATIONS

ABS_VOL=[link_ID](value) 
AVRCP_MEDIA [link_ID] (property: value)  #ARTIST: string TITLE: string ALBUM: string NUMBER: integer TOTAL_NUMBER: integer PLAYING_TIME(MS): integer
AVRCP_PLAY [link_ID] 
AVRCP_STOP [link_ID]
AVRCP_PAUSE [link_ID] 
AVRCP_FORWARD [link_ID] 
AVRCP_BACKWARD [link_ID] 
CALL (phone number) [link_ID]
CALL_ACTIVE[link_ID]
CALL_END[link_ID] 
CALL_INCOMING [link_IS]
CLOSE_ERROR (profile) [link_ID]
ERROR
HANGUP [link_ID]
LINK_LOSS [link_ID] (profile)
OPEN_ERROR (profile) [link_ID]
OPEN_OK (profile) [link_ID]
PAIR_ERROR (Bluetooth address)
PAIR_OK (Bluetooth address)
PAIR_PENDING 
READY
RING [link_ID] 
SIRI_STS=(status) 
SEND_OK








