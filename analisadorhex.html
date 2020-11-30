<style type="text/css">
#analise{
  font-family: monospace;
}
</style>

<b>Colar o Hex aqui:</b><br/>
<textarea id="hex"></textarea>
<button onclick="analisar()">Clique aqui!</button>
<ul>
<li>EOF : end of file</li>
<li>ESA : </li>

</ul>
<div id="analise">

</div>
<p>

</p>

<script>
  let decode = document.getElementById("analise");

  function calcular_checksum(data){
    return "00";
  }

  function analisar(){
    analise.innerText = "";
    let hex = document.getElementById("hex").value;

    let linhas = hex.split("\n");

    let size = 0;

    linhas.forEach( linha =>{
      // primeiro byte: byte count
      let byte_count = linha.substr(1,2) // 2 digitos
      // próximos 2 bytes: endereço
      let address = linha.substr(3,4)    // 4 dígitos
      let record_type = linha.substr(7,2) // record type
      let bc = parseInt(byte_count,16)

      let data = linha.substr(9, bc*2)

      let checksum = linha.substr(9 + bc*2,2);

      let rt = {
        "00" : "DATA",
        "01" : "EOF ",
        "02" : "ESA ",
        "03" : "SSA ",
        "04" : "ELA ",
        "05" : "SLA "
      }

      let addr_text = parseInt(address,16).toString().padStart(4,"_"),
          bc_text   = bc.toString().padStart(2,"_");
      let texto = `${bc_text} ${addr_text} ${rt[record_type]} ${data} (${data.length/2}) ${checksum}/${calcular_checksum(data)}`;
      size += bc;
      analise.innerText += texto + "\n";
    })
    analise.innerText += `Tamanho: ${size}`
  }
</script>
