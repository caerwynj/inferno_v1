Url: module
{
	PATH : con "/dis/lib/url.dis";

	# scheme ids
	NOSCHEME, HTTP, HTTPS, FTP, FILE, GOPHER, MAILTO, NEWS,
		NNTP, TELNET, WAIS, PROSPERO, UNKNOWN: con iota;

	# general url syntax:
	#    <scheme>://<user>:<passwd>@<host>:<port>/<path>;<params>?<query>#<fragment>
	#
	# relatives urls might omit some prefix of the above
	ParsedUrl: adt
	{
		scheme:	int;
		user:		string;
		passwd:	string;
		host:		string;
		port:		string;
		pstart:	string;	# what precedes <path>: either "/" or ""
		path:		string;
		params:	string;
		query:	string;
		frag:		string;

		makeabsolute: fn(url: self ref ParsedUrl, base: ref ParsedUrl);
		tostring: fn(url: self ref ParsedUrl) : string;
	};

	init: fn(S: String);
	makeurl: fn(s: string) : ref ParsedUrl;
};
