#2018/8/30 10:11
#����groupmember_t, group_t ��group_id ����
USE yourshadow;
ALTER TABLE groupmember_t MODIFY GROUP_ID VARCHAR(40) NOT NULL;
ALTER TABLE group_t MODIFY GROUP_ID VARCHAR(40) NOT NULL;