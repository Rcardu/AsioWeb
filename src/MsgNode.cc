#include "MsgNode.h"
#include "boost/asio/detail/socket_holder.hpp"
#include "deconst.h"

namespace ICEY {

RecvNode::RecvNode(int16_t max_len, int16_t msg_id)
    : MsgNode(max_len), m_msg_id(msg_id) {}

SendNode::SendNode(const char *msg, int16_t max_len, int16_t msg_id)
    : MsgNode(max_len + HEAD_TOTAL_LEN), m_msg_id(msg_id) {
  // 先发送id
  int16_t msg_id_host =
      boost::asio::detail::socket_ops::host_to_network_short(msg_id);
  memcpy(m_data, &msg_id_host, HEAD_ID_LEN);
  // 再发送长度
  int16_t max_len_host =
      boost::asio::detail::socket_ops::host_to_network_short(max_len);
  memcpy(m_data + HEAD_ID_LEN, &max_len_host, HEAD_DATA_LEN);
  // 最后发送消息体
  memcpy(m_data + HEAD_TOTAL_LEN, msg, max_len);
}
} // namespace ICEY
