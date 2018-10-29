#2018/8/30 10:11
#更新groupmember_t, group_t 的group_id 长度
USE yourshadow;
ALTER TABLE groupmember_t MODIFY GROUP_ID VARCHAR(40) NOT NULL;
ALTER TABLE group_t MODIFY GROUP_ID VARCHAR(40) NOT NULL;