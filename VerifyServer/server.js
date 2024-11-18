const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const defs_module = require('./defs')
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');
const config_module = require("./config")

async function GetVerifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        uniqueId = uuidv4();
        console.log("uniqueId is", uniqueId)
        let text_str =  '[Cutalk] 您的验证码为'+ uniqueId +'，请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: config_module.email_user,
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
    
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res", send_res)

        callback(null, { email: call.request.email,
            error: defs_module.ErrorCodes.Success
        }); 
        
 
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