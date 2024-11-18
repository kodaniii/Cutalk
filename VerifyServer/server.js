const grpc = require('@grpc/grpc-js')
const message_proto = require('./proto')
const defs_module = require('./defs')
const { v4: uuidv4 } = require('uuid');
const emailModule = require('./email');

async function GetVerifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        uniqueId = uuidv4();
        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: 'secondtonone1@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
    
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)

        callback(null, { email: call.request.email,
            error: defs_module.ErrorCodes.Success
        }); 
        
 
    }catch(error){
        console.log("catch error is ", error)

        callback(null, { email: call.request.email,
            error: defs_module.ErrorCodes.Exception
        }); 
    }
     
}

function main() {
    var server = new grpc.Server()
    //add message_proto service
    server.addService(message_proto.VerifyService.service, { GetVerifyCode: GetVerifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()