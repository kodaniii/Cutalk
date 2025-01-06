/*
 Navicat Premium Data Transfer

 Source Server         : cutalk-mysql
 Source Server Type    : MySQL
 Source Server Version : 80040 (8.0.40)
 Source Host           : 172.25.0.50:3307
 Source Schema         : Cutalk

 Target Server Type    : MySQL
 Target Server Version : 80040 (8.0.40)
 File Encoding         : 65001

 Date: 07/01/2025 00:16:11
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for friend
-- ----------------------------
DROP TABLE IF EXISTS `friend`;
CREATE TABLE `friend`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `self_id` int NOT NULL,
  `friend_id` int NOT NULL,
  `back` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `self_friend`(`self_id` ASC, `friend_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 54 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of friend
-- ----------------------------
INSERT INTO `friend` VALUES (34, 1, 11, '111');
INSERT INTO `friend` VALUES (35, 11, 1, '');
INSERT INTO `friend` VALUES (50, 1, 7, '');
INSERT INTO `friend` VALUES (51, 7, 1, '');
INSERT INTO `friend` VALUES (52, 1, 5, '222');
INSERT INTO `friend` VALUES (53, 5, 1, '');

-- ----------------------------
-- Table structure for friend_apply
-- ----------------------------
DROP TABLE IF EXISTS `friend_apply`;
CREATE TABLE `friend_apply`  (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `from_uid` int NOT NULL,
  `to_uid` int NOT NULL,
  `status` smallint NOT NULL DEFAULT 0,
  `apply_desc` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `back` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `from_to_uid`(`from_uid` ASC, `to_uid` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 54 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of friend_apply
-- ----------------------------
INSERT INTO `friend_apply` VALUES (51, 1, 11, 1, '', NULL);
INSERT INTO `friend_apply` VALUES (52, 5, 1, 1, '加个好友！', '');
INSERT INTO `friend_apply` VALUES (53, 7, 1, 1, '', '');

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `uid` int NOT NULL,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `email` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `pswd` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL,
  `nick` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `desc` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `sex` smallint NULL DEFAULT 0,
  `icon` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT ':/res/head/head_15.jpg',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 22 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of user
-- ----------------------------
INSERT INTO `user` VALUES (1, 1, 'apple', '1@cutalk.test', '0', 'testuid', NULL, 0, ':/res/head/head_1.jpg');
INSERT INTO `user` VALUES (2, 2, 'test', '1@1.test', '1234', NULL, NULL, 0, ':/res/head/head_2.jpg');
INSERT INTO `user` VALUES (3, 3, 'name1111', '1@1.ctest', '1111', NULL, NULL, 0, ':/res/head/head_3.jpg');
INSERT INTO `user` VALUES (4, 4, 'apple', '1@q.c', '1111', NULL, NULL, 0, ':/res/head/head_4.jpg');
INSERT INTO `user` VALUES (5, 5, 'test2', 'test@1.1', 'test2', NULL, NULL, 0, ':/res/head/head_5.jpg');
INSERT INTO `user` VALUES (6, 6, 'test3', 'test3@1.1', 'test', NULL, NULL, 0, ':/res/head/head_6.jpg');
INSERT INTO `user` VALUES (7, 7, 'test4', 'test4@1.a', '12345', NULL, NULL, 0, ':/res/head/head_7.jpg');
INSERT INTO `user` VALUES (8, 8, 'test5', 'test5@1.a', '12345', NULL, NULL, 0, ':/res/head/head_8.jpg');
INSERT INTO `user` VALUES (9, 9, 'test6', 'test6@1.a', '12345', NULL, NULL, 0, ':/res/head/head_9.jpg');
INSERT INTO `user` VALUES (10, 10, 'test7', 'test7@1.a', '12345', NULL, NULL, 0, ':/res/head/head_10.jpg');
INSERT INTO `user` VALUES (11, 11, 'xinzh', '436073092@qq.com', '774455', '测试号111', NULL, 0, ':/res/head/head_11.jpg');
INSERT INTO `user` VALUES (12, 12, 'orange', '2@1.c', '5555', '橘子', NULL, 0, ':/res/head/head_12.jpg');
INSERT INTO `user` VALUES (13, 13, 'helloworld', '10@1.c', '5555', NULL, NULL, 0, ':/res/head/head_13.jpg');
INSERT INTO `user` VALUES (14, 14, 'a880', '11@1.c', '777777', NULL, NULL, 0, ':/res/head/head_14.jpg');
INSERT INTO `user` VALUES (15, 15, 'ssaa', '12@1.c', '5555', NULL, NULL, 0, ':/res/head/head_15.jpg');
INSERT INTO `user` VALUES (16, 16, 'guet', '19@1.c', '5555', NULL, NULL, 0, ':/res/head/head_16.jpg');
INSERT INTO `user` VALUES (19, 19, '22233d', '1@1.c', '`777', NULL, NULL, 0, ':/res/head/head_17.jpg');
INSERT INTO `user` VALUES (20, 20, 'testgai11', '1@1.cc', '7770', NULL, NULL, 0, ':/res/head/head_1.jpg');
INSERT INTO `user` VALUES (21, 21, 'test222', 'test222@q.1', 'test', NULL, NULL, 0, ':/res/head/head_15.jpg');

-- ----------------------------
-- Table structure for user_id
-- ----------------------------
DROP TABLE IF EXISTS `user_id`;
CREATE TABLE `user_id`  (
  `id` int NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 10 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of user_id
-- ----------------------------
INSERT INTO `user_id` VALUES (20);

-- ----------------------------
-- Procedure structure for reg_user
-- ----------------------------
DROP PROCEDURE IF EXISTS `reg_user`;
delimiter ;;
CREATE PROCEDURE `reg_user`(IN `new_name` VARCHAR(255), 
    IN `new_email` VARCHAR(255), 
    IN `new_pswd` VARCHAR(255), 
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    
    -- 开始事务
    START TRANSACTION;

    -- 检查email是否已存在，即本邮箱是否被注册过
    IF EXISTS (SELECT 1 FROM `user` WHERE `email` = new_email) THEN
        SET result = -2; -- email已存在
        COMMIT;
    ELSE
        -- email不存在，检查用户名是否已存在
        IF EXISTS (SELECT 1 FROM `user` WHERE `name` = new_name) THEN
            SET result = -3; -- 用户名已存在
            COMMIT;
        ELSE
            -- email也不存在，更新user_id表
            UPDATE `user_id` SET `id` = `id` + 1;
            
            -- 获取更新后的id
            SELECT `id` INTO @new_id FROM `user_id`;
            
            -- 在user表中插入新记录
            INSERT INTO `user` (`uid`, `name`, `email`, `pswd`) VALUES (@new_id, new_name, new_email, new_pswd);
            -- 设置result为新插入的uid
            SET result = @new_id; -- 插入成功，返回新的uid
            COMMIT;
        
        END IF;
    END IF;
    
END
;;
delimiter ;

SET FOREIGN_KEY_CHECKS = 1;
