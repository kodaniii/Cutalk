const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const defs_module = require('./defs')
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');
const config_module = require("./config")
const redis_module = require("./redis")

async function GetVerifyCode(call, callback) {
    console.log("email is", call.request.email)
    try{
        /*检查是否生成过验证码 */
        //key: code_xxx@163.com
        //value: verifycode
        let query_res = await redis_module.GetRedis(defs_module.code_prefix + call.request.email);
        console.log("query_res is", query_res);
        let unique_id = query_res;
        //如果验证码没有生成过, 生成一个验证码
        if(query_res == null){
            unique_id = uuidv4();
            if (unique_id.length > 4) {
                unique_id = unique_id.substring(0, 4);
            }
            console.log("make a new unique_id", unique_id);
            let bres = await redis_module.SetRedisExpire(defs_module.code_prefix + call.request.email, unique_id, 180)
            if(!bres){
                callback(null, { email:  call.request.email,
                    error: defs_module.Errors.RedisErr
                });
                return;
            }
        }

        //uniqueId = uuidv4();
        console.log("unique_id is", unique_id)
        let text_str =  '[Cutalk] 您的验证码为'+ unique_id +'，请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: config_module.email_user,
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
    
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res", send_res)

        if(send_res){
            callback(null, { email: call.request.email,
                error: defs_module.ErrorCodes.Success
            }); 
            return;
        }
    
    }catch(error){
        console.log("catch error", error)

        callback(null, { email: call.request.email,
            error: defs_module.ErrorCodes.Exception
        }); 
    }
     
}

function main() {
    var server = new grpc.Server()
    //add message_proto service
    server.addService(message_proto.VerifyService.service, { GetVerifyCode: GetVerifyCode })
    server.bindAsync('127.0.0.1:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()