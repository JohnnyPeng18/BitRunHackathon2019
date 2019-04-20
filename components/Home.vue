<template>
    <div id="app">
        <!--导航条-->
        <el-menu :default-active="activeIndex2" class="el-menu-demo" mode="horizontal" @select="handleSelect"
                 background-color="#545c64" text-color="#fff" active-text-color="#ffd04b">
            <el-menu-item index="1">处理中心</el-menu-item>
            <el-submenu index="2">
                <template slot="title">我的工作台</template>
                <el-menu-item index="2-1">选项1</el-menu-item>
                <el-menu-item index="2-2">选项2</el-menu-item>
                <el-menu-item index="2-3">选项3</el-menu-item>
            </el-submenu>
        </el-menu>
        <br>
        <!--搜索框-->
        <el-row>
            <el-col :span="3" class="grid">
                <el-input v-model="input" placeholder="请输入内容" size="mini"></el-input>
            </el-col>
            <el-col :span="1" class="grid">
                <el-button type="success" icon="el-icon-search" size="mini">搜索</el-button>
            </el-col>
        </el-row>
        <br>
        <!--表格数据及操作-->
        <el-table :data="tableData" border style="width: 100%" stripe ref="multipleTable" tooltip-effect="dark">
            <!--勾选框-->
            <el-table-column type="selection" width="55">
            </el-table-column>
            <!--索引-->
            <el-table-column type="index" :index="indexMethod">
            </el-table-column>
            <el-table-column prop="name" label="名称" width="180" sortable>
            </el-table-column>
            <el-table-column prop="price" label="价格" width="180">
            </el-table-column>
            <el-table-column prop="last_time" label="最后出价时间">
            </el-table-column>
            <el-table-column prop="is_bought" label="状态">
             </el-table-column>

            <el-table-column label="primary" width="100">
                <template slot-scope="scope">
                    <el-button type="description" icon="el-icon-edit" size="mini">详情</el-button>
                </template>
            </el-table-column>
            <el-table-column label="danger" width="100">
                <template slot-scope="scope">
                    <el-button v-on:click="pay" type="danger" icon="el-icon-delete" size="mini">出价</el-button>
                </template>
            </el-table-column>
        </el-table>
        <br>
        <!--新增按钮-->
        <el-col :span="1" class="grid">
            <el-button type="sell" icon="el-icon-circle-plus-outline" size="mini" round>出售</el-button>
        </el-col>
        <!--全删按钮-->
        <el-col :span="1" class="grid">
            <el-button type="transaction" icon="el-icon-delete" size="mini" round>交易</el-button>
        </el-col>
        <br>
        <!--分页条-->
        <el-pagination background layout="prev, pager, next" :total="1000">
        </el-pagination>
    </div>
</template>
<script>
import GXClientFactory from "gxclient"
import Vue from 'vue';
import VueCookies from 'vue-cookies';
Vue.use(VueCookies);
let account_name_fake='hmmm123';
let private_key_fake='5KP7TdSYBoSAfoK4Kf5dsfCkGGovHzEMbycFLF7uiW6W73zt26c4';
var cookie=null
var arrItem=new Array();
    export default {
            data() {
                return {
                    //表格数据
                    tableData: [{
                        price: 1232131,
                        name: '王小虎',
                        last_time: '上海市普陀区金沙江路 1518 弄',
                        is_bought:'已出售'
                        }, {
                            price: '2016-05-04',
                            name: '王小虎',
                            last_time: '上海市普陀区金沙江路 1517 弄',
                        is_bought:'已出售'
                            }, {
                                price: '2016-05-01',
                                name: '王小虎',
                                last_time: '上海市普陀区金沙江路 1519 弄',
                        is_bought:'已出售'
                                }, {
                                    price: '2016-05-03',
                                    name: '王小虎',
                                    last_time: '上海市普陀区金沙江路 1516 弄',
                        is_bought:'已出售'
                                    },{price:3432532,
                                        name:'banana',
                                        last_time:'2019',
                        is_bought:'已出售'
                                        }],
                                    //查询输入框数据
                                    input: '',
                                    //导航条默认选项
                                    activeIndex: '1',
                                    activeIndex2: '1',
                                    account:'',
                                    key:''

                                    }
                },
            created:function(){
                var router=this.$router;
                var broadcast=true;
                alert("beginning");
                cookie=this.$cookies.get('User');
                console.log(cookie);
                console.log(cookie.account_name);
                console.log(cookie.private_key);
                let client=GXClientFactory.instance({keyProvider:cookie.private_key,account:cookie.account_name,network:"wss://testnet.gxchain.org"});
                if(!client.connected)
                {
                    alert("wrong wrong wrong wrong wrong")
                    // router.push({path:'/',name:'Login'})

                    }
                client.getTableObjects("auction8", "pubiterm").then(result=>{console.log("result:",result)
                    for(var i=0;result[i]!=null;i++)
                    {
                        arrItem[i]=result[i]

                        this.tableData[i].price=arrItem[i].final_price;

                        this.tableData[i].name=arrItem[i].name;

                    this.tableData[i].last_time=arrItem[i].last_pushtime;
                    if (arrItem[i].status==5){
                        this.tableData[i].is_bought='已出售'
                        }else{this.tableData[i].is_bought='未出售'}
                    }
                    
            console.log("arrItem:",arrItem);
            }
                    )
                },
            methods: {
                    handleSelect(key, keyPath) {
                        console.log(key, keyPath);
                        },
                    indexMethod(index) {
                        return index;
                        },
                    getParams(){
                        var router_account=this.$route.params.account;
                        //this.account=router_account
                        alert(router_account);
                        },
                    pay:function(event){
                        console.log("test:",this.tableData.price);
                        }

                    }
            }
</script>
<style>
    #app {
        font-family: Helvetica, sans-serif;
        text-align: center;
    }
</style>
