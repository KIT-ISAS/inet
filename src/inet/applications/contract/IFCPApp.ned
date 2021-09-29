package inet.applications.contract;

moduleinterface IFCPApp
{
    gates:
        input fcpIn @labels(FCPCommand/up);
        output fcpOut @labels(FCPCommand/down);
}