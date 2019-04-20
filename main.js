// The Vue build version to load with the `import` command
// (runtime-only or standalone) has been set in webpack.base.conf with an alias.
import Vue from 'vue'
import App from './App'
import router from './router'
import ElementUI from 'element-ui'
import VueCookies from 'vue-cookies'
import 'element-ui/lib/theme-chalk/index.css'
import './style/theme.css'
import './style/character.css'
import GXClientFactory from "gxclient";

Vue.use(ElementUI)
Vue.use(VueCookies)

const private_key="5KP7TdSYBoSAfoK4Kf5smCkGGovHzEMbdcFLF7uiW6W73zj26c4"
const account_name="yu340104"
let client=GXClientFactory.instance({keyProvider:private_key, account:account_name,network:"https://testnet.gxchain.org"});

let broadcast=true;
let contractName="auction8";
//Calling deposit method of bank contract
 client.callContract(contractName, "signin", {},null,broadcast).then(resp => {
    let transaction = broadcast ? resp[0].trx : resp;
    let txid = broadcast ? resp[0].id : "";
    console.log("JSON",txid, JSON.stringify(transaction));

/*
    client.getContractABI(contractName).then(abi => {
      //   describe the tables
        console.log("abi.table:",abi.tables);

        // load the data in the tables
        let promises = abi.tables.map(table => client.getTableObjects(contractName, table.name));
        Promise.all(promises).then(infos => {
         abi.tables.forEach((table, i) => {
              console.log("table:",table.name, infos[i]);
            });
        });
    });*/
});
//client.getTableObjects(contractName,'itermidraw'
 //       ).then(result=>{console.log(result);})//console.log("getTableObjects",myPromise);

Vue.config.productionTip = false
/* eslint-disable no-new */
new Vue({
  el: '#app',
  router,
  components: { App },
  template: '<App/>'
})
