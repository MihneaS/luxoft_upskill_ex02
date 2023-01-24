# luxoft_upskill_ex02

Some simple programs using the POSIX message queue. The server manages tokens that are requested through a POSIX message queue. The clients ask for a token and recieve the response on another message queue specific for each client.

## build
<p> Build with command </p>
<pre><code>make</code></pre>

## run

<p> To run type in one terminal</P>
<pre><code>./server</code></pre>
<p> and in another terminal type </p>
<pre><code>./client</code></pre>


## requirments

<p> The server manages token numbers, which could be seat numbers for a flight, or something similar. It is server's job to give a token number to a client on request. In a typical scenario, there might be multiple clients requesting the server for token numbers. The server's message queue name is known to clients. Each client has its own message queue, in which server posts responses. When a client sends a request, it sends its message queue name. The server opens client's message queue and sends its response. The client picks up the response from its message queue and reads the token number in it. </p>
