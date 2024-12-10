CREATE DEFINER=`root`@`%` PROCEDURE `reg_user`(
    IN `new_name` VARCHAR(255), 
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