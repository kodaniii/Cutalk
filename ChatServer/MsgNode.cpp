#include "MsgNode.h"
RecvNode::RecvNode(short max_len, short msg_type_id)
	: MsgNode(max_len), _msg_type_id(msg_type_id){

}

//消息存储到SendNode::_data，首先存储id和len，然后才是消息体
SendNode::SendNode(const char *msg, short max_len, short msg_type_id)
	: MsgNode(max_len + HEAD_TOTAL_LEN), _msg_type_id(msg_type_id){
	//转为网络字节序
	short msg_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_type_id);
	short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);

	memcpy(_data, &msg_id_host, HEAD_ID_LEN);
	memcpy(_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
	memcpy(_data + HEAD_ID_LEN + HEAD_DATA_LEN, msg, max_len);
}
