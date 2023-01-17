# luxoft_upskill_ex02

Some simple programs using the POSIX message queue. The server openes a queue for writing and waits. Then a script starts the clients whitch read from that queue.
At the end, the server closes the message queue.

## build
<p> build with command </p>
<pre><code>make</code></pre>

## run

<p> To run type </P>
<pre><code>./server</code></pre>
<p> After the server initialised, in another console type </p>
<pre><code>./start_clients.sh</code></pre>
<p> Afterwards, press enter in the first console start exchanging messages. When all clients are done, press enter again in the first console to close the server</p>
