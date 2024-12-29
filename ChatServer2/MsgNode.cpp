#include "MsgNode.h"
RecvNode::RecvNode(short max_len, short msg_type_id)
	: MsgNode(max_len), _msg_type_id(msg_type_id){

}

//��Ϣ�洢��SendNode::_data�����ȴ洢id��len��Ȼ�������Ϣ��
SendNode::SendNode(const char *msg, short max_len, short msg_type_id)
	: MsgNode(max_len + HEAD_TOTAL_LEN), _msg_type_id(msg_type_id){
	//תΪ�����ֽ���
	short msg_type_id_host = boost::asio::detail::socket_ops::host_to_network_short(msg_type_id);
	short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);

	memcpy(_data, &msg_type_id_host, HEAD_ID_LEN);
	memcpy(_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
	memcpy(_data + HEAD_ID_LEN + HEAD_DATA_LEN, msg, max_len);
}