// -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

//-----------------------------------------------------------------------------
// State.h
// (c) Marc Schoenauer, Maarten Keijzer and GeNeura Team, 2000
/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contact: todos@geneura.ugr.es, http://geneura.ugr.es
             Marc.Schoenauer@polytechnique.fr
             mkeijzer@dhi.dk
 */
//-----------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <fstream>
#include <sstream>

#include "State.h"
#include "Object.h"
#include "Persistent.h"

namespace dim
{
    namespace core
    {

    using namespace std;

    void removeComment(string& str, string comment)
    {
	string::size_type pos = str.find(comment);

	if (pos != string::npos)
	    {
		str.erase(pos, str.size());
	    }
    }

    bool is_section(const string& str, string& name)
    {
	string::size_type pos = str.find("\\section{");

	if (pos == string::npos)
	    return false;
	//else

	string::size_type end = str.find("}");

	if (end == string::npos)
	    return false;
	// else

	name = str.substr(pos + 9, end-9);

	return true;
    }

    State::~State(void)
    {
	for (unsigned i = 0; i < ownedObjects.size(); ++i)
	    {
		delete ownedObjects[i];
	    }
    }

    void State::registerObject(Persistent& registrant)
    {
	string name = createObjectName(dynamic_cast<Object*>(&registrant));

	pair<ObjectMap::iterator,bool> res = objectMap.insert(make_pair(name, &registrant));

	if (res.second == true)
	    {
		creationOrder.push_back(res.first);
	    }
	else
	    {
		throw logic_error("Interval error: object already present in the state");
	    }
    }

    void State::load(const string& _filename)
    {
	ifstream is (_filename.c_str());

	if (!is)
	    {
		string str = "Could not open file " + _filename;
		throw runtime_error(str);
	    }

	load(is);
    }

    void State::load(std::istream& is)
    {
	string str;
	string name;

	getline(is, str);

	if (is.fail())
	    {
		string str = "Error while reading stream";
		throw runtime_error(str);
	    }

	while(! is.eof())
	    { // parse section header
		if (is_section(str, name))
		    {
			string fullString;
			ObjectMap::iterator it = objectMap.find(name);

			if (it == objectMap.end())
			    { // ignore
				while (getline(is, str))
				    {
					if (is_section(str, name))
					    break;
				    }
			    }
			else
			    {

				Persistent* object = it->second;

				// now we have the object, get lines, remove comments etc.

				string fullstring;

				while (getline(is, str))
				    {
					if (is.eof())
					    throw runtime_error("No section in load file");
					if (is_section(str, name))
					    break;

					removeComment(str, getCommentString());
					fullstring += str + "\n";
				    }
				istringstream the_stream(fullstring);
				object->readFrom(the_stream);
			    }
		    }
		else // if (is_section(str, name)) - what if file empty
		    {
			getline(is, str);	// try next line!
			//	    if (is.eof())
			//	      throw runtime_error("No section in load file");
		    }
	    }

    }

    void State::save(const string& filename) const
    { // saves in order of insertion
	ofstream os(filename.c_str());

	if (!os)
	    {
		string msg = "Could not open file: " + filename + " for writing!";
		throw runtime_error(msg);
	    }

	save(os);
    }

    void State::save(std::ostream& os) const
    { // saves in order of insertion
	for (vector<ObjectMap::iterator>::const_iterator it = creationOrder.begin(); it != creationOrder.end(); ++it)
	    {
		os << "\\section{" << (*it)->first << "}\n";
		(*it)->second->printOn(os);
		os << '\n';
	    }
    }

	string State::createObjectName(Object* obj)
	{
	    if (obj == 0)
		{
		    ostringstream os;
		    os << objectMap.size();
		    return os.str();
		}
	    // else

	    string name = obj->className();
	    ObjectMap::const_iterator it = objectMap.find(name);

	    unsigned count = 1;
	    while (it != objectMap.end())
		{
		    ostringstream os;
		    os << obj->className().c_str() << count++;
		    name = os.str();
		    it = objectMap.find(name);
		}

	    return name;
	}

    } // !core
} // !dim
