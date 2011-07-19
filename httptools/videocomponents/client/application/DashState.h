//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef DASHSTATE_H_
#define DASHSTATE_H_

class DashController;
class DashState {
public:
	virtual void connectionInitialized(DashController *, int connId);
	virtual void connectionFailed(DashController *, int connId);
	virtual void jobFinished(DashController *, int jobId);
	virtual void jobProgress(DashController *, int jobId);
	virtual void handleCallback(DashController *);
protected:
	DashState();
	void ChangeState(DashController *, DashState *);
};

/* State where nothing is done (i.e., before, after watching) */
class DashIdle: public DashState {
public:
	static DashState * Instance();
	virtual void handleCallback(DashController *app);
protected:
	DashIdle();
private:
	static DashState * _instance;
};

/* State where client is in the connecting phase */
class DashConnect: public DashState {
public:
	static DashState * Instance();
	virtual void connectionInitialized(DashController *, int connId);
	virtual void connectionFailed(DashController *, int connId);
protected:
	DashConnect();
private:
	static DashState * _instance;
};

class DashRequest: public DashState {
public:
	static DashState * Instance();
	virtual void connectionFailed(DashController *, int connId);
	virtual void jobFinished(DashController *, int jobId);
	virtual void jobProgress(DashController *, int jobId);
protected:
	DashRequest();
private:
	static DashState * _instance;
};

class DashBufferFull: public DashState {
public:
	static DashState * Instance();
	virtual void connectionFailed(DashController *, int connId);
	virtual void handleCallback(DashController *);
protected:
	DashBufferFull();
private:
	static DashState * _instance;
};
#endif /* DASHSTATE_H_ */
