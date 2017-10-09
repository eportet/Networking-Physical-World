'use strict';
const http = require("http");
var PASSWORD = process.env.PASSWORD

// Route the incoming request based on type (LaunchRequest, IntentRequest,
// etc.) The JSON body of the request is provided in the event parameter.
exports.handler = function (event, context) {
	try {
		console.log("event.session.application.applicationId=" + event.session.application.applicationId);

		/**
		 * Uncomment this if statement and populate with your skill's application ID to
		 * prevent someone else from configuring a skill that sends requests to this function.
		 */

		 if (event.session.application.applicationId !== "amzn1.ask.skill.b70028a8-dcd2-405b-b049-ceb4ef3ecb2c") {
			context.fail("Invalid Application ID");
		 }

		 if (event.session.new) {
			onSessionStarted({requestId: event.request.requestId}, event.session);
		 }

		 if (event.request.type === "LaunchRequest") {
			onLaunch(event.request,
				event.session,
				function callback(sessionAttributes, speechletResponse) {
					context.succeed(buildResponse(sessionAttributes, speechletResponse));
				});
		 } else if (event.request.type === "IntentRequest") {
			onIntent(event.request,
				event.session,
				function callback(sessionAttributes, speechletResponse) {
					context.succeed(buildResponse(sessionAttributes, speechletResponse));
				});
		 } else if (event.request.type === "SessionEndedRequest") {
			onSessionEnded(event.request, event.session);
			context.succeed();
		 }
		} catch (e) {
			context.fail("Exception: " + e);
		}
	};

/**
 * Called when the session starts.
 */
 function onSessionStarted(sessionStartedRequest, session) {
	// add any session init logic here
}

/**
 * Called when the user invokes the skill without specifying what they want.
 */
 function onLaunch(launchRequest, session, callback) {
	getWelcomeResponse(callback)
 }

/**
 * Called when the user specifies an intent for this skill.
 */
 function onIntent(intentRequest, session, callback) {

	var intent = intentRequest.intent
	var intentName = intentRequest.intent.name;

	// dispatch custom intents to handlers here
	if (intentName == "ToggleIntent") {
		handleToggleResponse(intent, session, callback)
	} else if (intentName == "ToggleAllIntent") {
		handleToggleAllResponse(intent, session, callback)
	} else if (intentName == "StatusIntent") {
		handleStatusResponse(intent, session, callback)
	} else if (intentName == "AMAZON.YesIntent") {
		handleYesResponse(intent, session, callback)
	} else if (intentName == "AMAZON.NoIntent") {
		handleNoResponse(intent, session, callback)
	} else if (intentName == "AMAZON.HelpIntent") {
		handleGetHelpRequest(intent, session, callback)
	} else if (intentName == "AMAZON.StopIntent") {
		handleFinishSessionRequest(intent, session, callback)
	} else if (intentName == "AMAZON.CancelIntent") {
		handleFinishSessionRequest(intent, session, callback)
	} else {
		throw "Invalid intent"
	}
}

/**
 * Called when the user ends the session.
 * Is not called when the skill returns shouldEndSession=true.
 */
 function onSessionEnded(sessionEndedRequest, session) {

 }

// ------- Skill specific logic -------

function getWelcomeResponse(callback) {
	var speechOutput = "Which device would you like to toggle?"

	var reprompt = "Which device would you like to toggle?"

	var header = "Toggle"

	var shouldEndSession = false

	var sessionAttributes = {
		"speechOutput" : speechOutput,
		"repromptText" : reprompt
	}

	callback(sessionAttributes, buildSpeechletResponse(header, speechOutput, reprompt, shouldEndSession))

}

function handleToggleResponse(intent, session, callback) {
	var id = intent.slots.number.value

	http.get('http://128.197.175.187:9000/toggle?id=' + id + '&password=' + PASSWORD, (res) => {
		const statusCode = res.statusCode;
		const contentType = res.headers['content-type'];

		let error;
		if (statusCode !== 200) {
			error = new Error('Request Failed.\n' +
				`Status Code: ${statusCode}`);
		} 
		if (error) {
			console.log(error.message);
			// consume response data to free up memory
			res.resume();
			this.emit(':tell', "There was an error");
		}
		
		var speechOutput = "You toggled device ID " + id + " Do you want to do anything else?" 

		var repromptText = "Do you want to do anything else?"
		
		var header = "Toggle ID"

		var shouldEndSession = false

		callback(session.attributes, buildSpeechletResponse(header, speechOutput, repromptText, shouldEndSession))

		res.setEncoding('utf8');
		let rawData = '';
		res.on('data', (chunk) => rawData += chunk);
		res.on('end', () => {
			try {
				const parsedData = JSON.parse(rawData);
				console.log(parsedData);
			} catch (e) {
				console.log(e.message);
			}
		});
	}).on('error', (e) => {
		console.log(`Got error: ${e.message}`);
		this.emit(':tellWithCard', "Error", this.t('SKILL_NAME'), randomFact);
	});
}

function handleToggleAllResponse(intent, session, callback) {

	http.get('http://128.197.175.187:9000/toggle&password=' + PASSWORD, (res) => {
		const statusCode = res.statusCode;
		const contentType = res.headers['content-type'];

		let error;
		if (statusCode !== 200) {
			error = new Error('Request Failed.\n' +
				`Status Code: ${statusCode}`);
		} 
		if (error) {
			console.log(error.message);
			// consume response data to free up memory
			res.resume();
			this.emit(':tell', "There was an error");
		}
		var speechOutput = "You toggled all devices." + "Do you want to do anything else?"

		var repromptText = "Do you want to do anything else?"
		var header = "Toggle All"

		var shouldEndSession = false

		callback(session.attributes, buildSpeechletResponse(header, speechOutput, repromptText, shouldEndSession))

		res.setEncoding('utf8');
		let rawData = '';
		res.on('data', (chunk) => rawData += chunk);
		res.on('end', () => {
			try {
				const parsedData = JSON.parse(rawData);
				console.log(parsedData);
			} catch (e) {
				console.log(e.message);
			}
		});
	}).on('error', (e) => {
		console.log(`Got error: ${e.message}`);
		this.emit(':tellWithCard', "Error", this.t('SKILL_NAME'), randomFact);
	});
}

function handleStatusResponse(intent, session, callback) {
	var id = intent.slots.number.value    
	
	http.get('http://128.197.175.187:9000/get?id=' + id + '&password=' + PASSWORD, (res) => {
		const statusCode = res.statusCode;
		const contentType = res.headers['content-type'];

		let error;
		if (statusCode !== 200) {
			error = new Error('Request Failed.\n' +
				`Status Code: ${statusCode}`);
		} 
		if (error) {
			console.log(error.message);
			// consume response data to free up memory
			res.resume();
			this.emit(':tell', "There was an error");
		}

		res.setEncoding('utf8');
		let rawData = '';
		res.on('data', (chunk) => rawData += chunk);
		res.on('end', () => {
			try {
				var speechOutput = rawData + "Do you want to do anything else?"

				var repromptText = "Do you want to do anything else?"
				var header = "Status"

				var shouldEndSession = false

				callback(session.attributes, buildSpeechletResponse(header, speechOutput, repromptText, shouldEndSession))
			} catch (e) {
				console.log(e.message);
			}
		});
	}).on('error', (e) => {
		console.log(`Got error: ${e.message}`);
		this.emit(':tellWithCard', "Error", this.t('SKILL_NAME'), randomFact);
	});
}

function handleYesResponse(intent, session, callback) {
	var speechOutput = "Which device would you like to toggle? You can also toggle all devices!"
	var repromptText = speechOutput
	var shouldEndSession = false

	callback(session.attributes, buildSpeechletResponseWithoutCard(speechOutput, repromptText, shouldEndSession))
}

function handleNoResponse(intent, session, callback) {
	handleFinishSessionRequest(intent, session, callback)
}

function handleGetHelpRequest(intent, session, callback) {
	// Ensure that session.attributes has been initialized
	if (!session.attributes) {
		session.attributes = {};
	}

	var speechOutput = "You can toggle the LED on a single device." + 
	" Or, you can toggle all devices." +
	" What do you want to do? "

	var repromptText = speechOutput

	var shouldEndSession = false

	callback(session.attributes, buildSpeechletResponseWithoutCard(speechOutput, repromptText, shouldEndSession))

}

function handleFinishSessionRequest(intent, session, callback) {
	// End the session with a "Good bye!" if the user wants to quit the game
	callback(session.attributes,
		buildSpeechletResponseWithoutCard("Closing the application!", "", true));
}


// ------- Helper functions to build responses for Alexa -------


function buildSpeechletResponse(title, output, repromptText, shouldEndSession) {
	return {
		outputSpeech: {
			type: "PlainText",
			text: output
		},
		card: {
			type: "Simple",
			title: title,
			content: output
		},
		reprompt: {
			outputSpeech: {
				type: "PlainText",
				text: repromptText
			}
		},
		shouldEndSession: shouldEndSession
	};
}

function buildSpeechletResponseWithoutCard(output, repromptText, shouldEndSession) {
	return {
		outputSpeech: {
			type: "PlainText",
			text: output
		},
		reprompt: {
			outputSpeech: {
				type: "PlainText",
				text: repromptText
			}
		},
		shouldEndSession: shouldEndSession
	};
}

function buildResponse(sessionAttributes, speechletResponse) {
	return {
		version: "1.0",
		sessionAttributes: sessionAttributes,
		response: speechletResponse
	};
}

function capitalizeFirst(s) {
	return s.charAt(0).toUpperCase() + s.slice(1)
}